#include <canvascontext/include/image-context.h>

using namespace v8;

Local<Object> Image::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Image"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto, "load", Load);

  return scope.Escape(Nan::GetFunction(ctor).ToLocalChecked());
}

unsigned int Image::GetWidth() {
  if (image) {
    return image->width();
  } else {
    return 0;
  }
}

unsigned int Image::GetHeight() {
  if (image) {
    return image->height();
  } else {
    return 0;
  }
}

unsigned int Image::GetNumChannels() {
  return 4;
}

/* unsigned char *Image::GetData() {
  if (image) {
    return image->getData().getData();
  } else {
    return nullptr;
  }
} */

void Image::Load(Local<ArrayBuffer> arrayBuffer, size_t byteOffset, size_t byteLength, Local<Function> cbFn) {
  if (!this->loading) {
    unsigned char *buffer = (unsigned char *)arrayBuffer->GetContents().Data() + byteOffset;
    sk_sp<SkData> extendedData = SkData::MakeUninitialized(byteLength + 1);
    memcpy((unsigned char *)extendedData->data(), buffer, byteLength);
    sk_sp<SkData> data = SkData::MakeWithoutCopy(buffer, byteLength);

    this->cbFn.Reset(cbFn);
    this->loading = true;
    this->hasCbFn = !cbFn.IsEmpty();
    
    auto loadFn = [this, extendedData, data]() -> void {
      SkBitmap bitmap;
      bool ok = DecodeDataToBitmap(data, &bitmap);

      if (ok) {
        bitmap.setImmutable();
        this->image = SkImage::MakeFromBitmap(bitmap);
      } else {
        ((char *)extendedData->data())[extendedData->size() - 1] = 0; // NUL-terminate

        NSVGimage *svgImage = nsvgParse((char *)extendedData->data(), "px", 96);
        if (svgImage != nullptr) {
          if (svgImage->width > 0 && svgImage->height > 0 && svgImage->shapes != nullptr) {
            int w = svgImage->width;
            int h = svgImage->height;
            unsigned char *address = (unsigned char *)malloc(w * h * 4);

            // Create, use, and destroy rasterizer. Before was often
            // segfaulting on I-Frames when the there was a single static
            // rasterizer instance.
            NSVGrasterizer *imageContextSvgRasterizer = nsvgCreateRasterizer();
            nsvgRasterize(imageContextSvgRasterizer, svgImage, 0, 0, 1, address, w, h, w * 4);
            nsvgDeleteRasterizer(imageContextSvgRasterizer);

            SkImageInfo info = SkImageInfo::Make(w, h, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
            SkPixmap pixmap(info, address, w * 4);

            SkBitmap bitmap;
            bool ok = bitmap.installPixels(pixmap);
            if (ok) {
              bitmap.setImmutable();
              this->image = SkImage::MakeFromBitmap(bitmap);
            } else {
              this->error = "failed to install svg pixels";

              free(address);
            }
          } else {
            this->error = "invalid svg parameters";
          }
        } else {
          this->error = "unknown image type";
          // throw ImageLoadingException(stbi_failure_reason());
        }
      }

      if (!this->hasCbFn) {
        uv_sem_post(&this->sem);
      }
    };

    if (this->hasCbFn) {
      threadpool::ThreadPool *threadPool = threadpool::getWindowThreadPool();
      threadPool->queueWork(loadFn, [this]() -> void {
        Nan::HandleScope scope;
        
        Local<Object> asyncObject = Nan::New<Object>();
        AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "Image::Load");

        Local<Function> cbFn = Nan::New(this->cbFn);
        Local<String> arg0 = Nan::New<String>(this->error).ToLocalChecked();
        Local<Value> argv[] = {
          arg0,
        };
        asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);

        this->cbFn.Reset();
        this->error = "";
      });
    } else {
      uv_sem_init(&this->sem, 0);
      
      loadFn();

      uv_sem_wait(&this->sem);
      uv_sem_destroy(&this->sem);

      if (this->error.length() > 0) {
        Nan::ThrowError(this->error.c_str());
      }
    }
  } else {
    Nan::ThrowError("already loading");
  }
}

/* void Image::Set(canvas::Image *image) {
  this->image = image->image;
} */

NAN_METHOD(Image::New) {
  Nan::HandleScope scope;

  Local<Object> imageObj = info.This();

  Image *image = new Image();
  image->Wrap(imageObj);

  Nan::SetAccessor(imageObj, JS_STR("width"), WidthGetter);
  Nan::SetAccessor(imageObj, JS_STR("height"), HeightGetter);
  Nan::SetAccessor(imageObj, JS_STR("data"), DataGetter);

  info.GetReturnValue().Set(imageObj);
}

NAN_GETTER(Image::WidthGetter) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(info.This());
  info.GetReturnValue().Set(JS_INT(image->GetWidth()));
}

NAN_GETTER(Image::HeightGetter) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(info.This());
  info.GetReturnValue().Set(JS_INT(image->GetHeight()));
}

NAN_GETTER(Image::DataGetter) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(info.This());
  if (image->dataArray.IsEmpty()) {
    if (image->image) {
      SkPixmap pixmap;
      bool ok = image->image->peekPixels(&pixmap);

      if (ok) {
        unsigned int width = image->GetWidth();
        unsigned int height = image->GetHeight();
        Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), (void *)pixmap.addr(), width * height * 4); // XXX link lifetime

        Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
        image->dataArray.Reset(uint8ClampedArray);
      } else {
        return info.GetReturnValue().Set(Nan::Null());
      }
    } else {
      return info.GetReturnValue().Set(Nan::Null());
    }
  }

  info.GetReturnValue().Set(Nan::New(image->dataArray));
}

NAN_METHOD(Image::Load) {
  // Nan::HandleScope scope;

  if (info[0]->IsArrayBuffer()) {
    Image *image = ObjectWrap::Unwrap<Image>(info.This());

    Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(info[0]);
    Local<Function> cbFn;
    if (info[1]->IsFunction()) {
      cbFn = Local<Function>::Cast(info[1]);
    }

    image->Load(arrayBuffer, 0, arrayBuffer->ByteLength(), cbFn);
  } else if (info[0]->IsTypedArray()) {
    Image *image = ObjectWrap::Unwrap<Image>(info.This());

    Local<ArrayBufferView> arrayBufferView = Local<ArrayBufferView>::Cast(info[0]);
    Local<ArrayBuffer> arrayBuffer = arrayBufferView->Buffer();
    Local<Function> cbFn;
    if (info[1]->IsFunction()) {
      cbFn = Local<Function>::Cast(info[1]);
    }

    image->Load(arrayBuffer, arrayBufferView->ByteOffset(), arrayBufferView->ByteLength(), cbFn);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

Image::Image () : loading(false), hasCbFn(false) {}
Image::~Image () {}

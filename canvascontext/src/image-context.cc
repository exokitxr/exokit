#include <canvascontext/include/image-context.h>

using namespace v8;

static NSVGrasterizer *imageContextSvgRasterizer = nsvgCreateRasterizer();

Handle<Object> Image::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Image"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto,"load", LoadMethod);

  return scope.Escape(ctor->GetFunction());
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

bool Image::Load(const unsigned char *buffer, size_t size) {
  sk_sp<SkData> data = SkData::MakeWithoutCopy(buffer, size);
  SkBitmap bitmap;
  bool ok = DecodeDataToBitmap(data, &bitmap);

  if (ok) {
    bitmap.setImmutable();
    image = SkImage::MakeFromBitmap(bitmap);

    return true;
  } else {
    unique_ptr<char[]> svgString(new char[size + 1]);
    memcpy(svgString.get(), buffer, size);
    svgString[size] = 0;

    NSVGimage *svgImage = nsvgParse(svgString.get(), "px", 96);
    if (svgImage != nullptr) {
      if (svgImage->width > 0 && svgImage->height > 0 && svgImage->shapes != nullptr) {
        int w = svgImage->width;
        int h = svgImage->height;
        unsigned char *address = (unsigned char *)malloc(w * h * 4);
        nsvgRasterize(imageContextSvgRasterizer, svgImage, 0, 0, 1, address, w, h, w * 4);

        SkImageInfo info = SkImageInfo::Make(w, h, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
        SkPixmap pixmap(info, address, w * 4);

        SkBitmap bitmap;
        bool ok = bitmap.installPixels(pixmap);
        if (ok) {
          bitmap.setImmutable();
          image = SkImage::MakeFromBitmap(bitmap);

          // std::cout << "image decode ok" << "\n";

          return true;
        } else {
          free(address);

          return false;
        }
      } else {
        // std::cout << "image decode fail 1" << "\n";

        return false;
      }
    } else {
      // std::cout << "image decode fail 2" << "\n";
      // throw ImageLoadingException(stbi_failure_reason());
      return false;
    }
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
  }

  info.GetReturnValue().Set(Nan::New(image->dataArray));
}

NAN_METHOD(Image::LoadMethod) {
  Nan::HandleScope scope;

  if (info[0]->IsArrayBuffer()) {
    Image *image = ObjectWrap::Unwrap<Image>(info.This());

    Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(info[0]);

    bool ok = image->Load((unsigned char *)arrayBuffer->GetContents().Data(), arrayBuffer->ByteLength());
    info.GetReturnValue().Set(JS_BOOL(ok));
  } else if (info[0]->IsTypedArray()) {
    Image *image = ObjectWrap::Unwrap<Image>(info.This());

    Local<ArrayBufferView> arrayBufferView = Local<ArrayBufferView>::Cast(info[0]);
    Local<ArrayBuffer> arrayBuffer = arrayBufferView->Buffer();

    bool ok = image->Load((unsigned char *)arrayBuffer->GetContents().Data() + arrayBufferView->ByteOffset(), arrayBufferView->ByteLength());
    info.GetReturnValue().Set(JS_BOOL(ok));
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

Image::Image () {}
Image::~Image () {}

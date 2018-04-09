#include <canvascontext/include/imageBitmap-context.h>

using namespace v8;
using namespace node;

Handle<Object> ImageBitmap::Initialize(Isolate *isolate, Local<Value> imageCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("ImageBitmap"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

unsigned int ImageBitmap::GetWidth() {
  return bitmap.width();
}

unsigned int ImageBitmap::GetHeight() {
  return bitmap.height();
}

unsigned int ImageBitmap::GetNumChannels() {
  return 4;
}

/* unsigned char *ImageBitmap::GetData() {
  if (imageData != nullptr) {
    return imageData->getData();
  } else {
    return nullptr;
  }
} */

/* void ImageBitmap::Set(canvas::ImageData *imageData) {
  if (this->imageData != nullptr) {
    delete this->imageData;
  }
  this->imageData = imageData;
} */

NAN_METHOD(ImageBitmap::New) {
  Nan::HandleScope scope;

  Local<Object> imageBitmapObj = info.This();

  if (info[0]->IsObject() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber() && info[4]->IsNumber() && info[5]->IsBoolean()) {
    Image *image = ObjectWrap::Unwrap<Image>(info[0]->ToObject());
    int x = info[1]->Int32Value();
    int y = info[2]->Int32Value();
    unsigned int width = info[3]->Uint32Value();
    unsigned int height = info[4]->Uint32Value();
    bool flipY = info[5]->BooleanValue();

    SkImageInfo info = SkImageInfo::Make(width, height, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
    unsigned char *address = (unsigned char *)malloc(width * height * 4);
    SkPixmap pixmap(info, address, width * 4);
    bool ok = image->image->scalePixels(pixmap, SkFilterQuality::kNone_SkFilterQuality);

    if (ok) {
      if (!flipY) { // the default representation is flipped; flip iff the user did not want this
        unique_ptr<char[]> line(new char[width * 4]);

        for (size_t i = 0; i < height / 2; i++) {
          unsigned char *topAddress = address + (i * width * 4);
          unsigned char *bottomAddress = address + ((height - 1 - i) * width * 4);
          memcpy(line.get(), topAddress, width * 4);
          memcpy(topAddress, bottomAddress, width * 4);
          memcpy(bottomAddress, line.get(), width * 4);
        }
      }

      SkBitmap bitmap;
      bool ok = bitmap.installPixels(pixmap);
      if (ok) {
        ImageBitmap *imageBitmap = new ImageBitmap(bitmap);
        imageBitmap->Wrap(imageBitmapObj);
      } else {
        free(address);

        return Nan::ThrowError("Failed to install pixels");
      }
    } else {
      free(address);

      return Nan::ThrowError("Failed to read pixels");
    }
  } else {
    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsArrayBufferView()) {
      unsigned int width = info[0]->Uint32Value();
      unsigned int height = info[1]->Uint32Value();
      Local<ArrayBufferView> dataValue = Local<ArrayBufferView>::Cast(info[2]);
      char *data = (char *)dataValue->Buffer()->GetContents().Data() + dataValue->ByteOffset();

      unsigned char *address = (unsigned char *)malloc(width * height * 4);
      memcpy(address, data, width * height * 4);

      SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
      SkPixmap pixmap(info, address, width * 4);

      SkBitmap bitmap;
      bool ok = bitmap.installPixels(pixmap);
      if (ok) {
        ImageBitmap *imageBitmap = new ImageBitmap(bitmap);
        imageBitmap->Wrap(imageBitmapObj);
      } else {
        free(address);

        return Nan::ThrowError("Failed to install pixels");
      }
    } else {
      return Nan::ThrowError("Invalid arguments");
    }
  }

  Nan::SetAccessor(imageBitmapObj, JS_STR("width"), WidthGetter);
  Nan::SetAccessor(imageBitmapObj, JS_STR("height"), HeightGetter);
  Nan::SetAccessor(imageBitmapObj, JS_STR("data"), DataGetter);

  info.GetReturnValue().Set(imageBitmapObj);
}

NAN_GETTER(ImageBitmap::WidthGetter) {
  Nan::HandleScope scope;

  ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(info.This());
  info.GetReturnValue().Set(JS_INT(imageBitmap->GetWidth()));
}

NAN_GETTER(ImageBitmap::HeightGetter) {
  Nan::HandleScope scope;

  ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(info.This());
  info.GetReturnValue().Set(JS_INT(imageBitmap->GetHeight()));
}

NAN_GETTER(ImageBitmap::DataGetter) {
  Nan::HandleScope scope;

  ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(info.This());
  if (imageBitmap->dataArray.IsEmpty()) {
    SkPixmap pixmap;
    bool ok = imageBitmap->bitmap.peekPixels(&pixmap);

    if (ok) {
      unsigned int width = imageBitmap->GetWidth();
      unsigned int height = imageBitmap->GetHeight();
      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), (void *)pixmap.addr(), width * height * 4); // XXX link lifetime

      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      imageBitmap->dataArray.Reset(uint8ClampedArray);
    } else {
      return info.GetReturnValue().Set(Nan::Null());
    }
  }

  info.GetReturnValue().Set(Nan::New(imageBitmap->dataArray));
}

ImageBitmap::ImageBitmap() {}
/* ImageBitmap::ImageBitmap(unsigned int width, unsigned int height, unsigned char *data) {
  this->bitmap = bitmap;
}
ImageBitmap::ImageBitmap(Image *image, int x, int y, unsigned int width, unsigned int height, bool flipY) :
  imageData(image->image->getData().crop(x, y, width, height, flipY).release()) {} */
ImageBitmap::ImageBitmap(const SkBitmap &bitmap) {
  this->bitmap = bitmap;
}
ImageBitmap::~ImageBitmap () {}

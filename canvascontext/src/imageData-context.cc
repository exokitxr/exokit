#include <canvascontext/include/imageData-context.h>

using namespace v8;
using namespace node;

Handle<Object> ImageData::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("ImageData"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  return scope.Escape(ctor->GetFunction());
}

unsigned int ImageData::GetWidth() {
  return bitmap.width();
}

unsigned int ImageData::GetHeight() {
  return bitmap.height();
}

unsigned int ImageData::GetNumChannels() {
  return 4;
}

/* unsigned char *ImageData::GetData() {
  return imageData->getData();
} */

/* void ImageData::Set(canvas::ImageData *imageData) {
  this->bitmap = imageData->bitmap;
} */

NAN_METHOD(ImageData::New) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    Local<Object> imageDataObj = info.This();

    unsigned int width = info[0]->Uint32Value();
    unsigned int height = info[1]->Uint32Value();
    ImageData *imageData = new ImageData(width, height);
    imageData->Wrap(imageDataObj);

    Nan::SetAccessor(imageDataObj, JS_STR("width"), WidthGetter);
    Nan::SetAccessor(imageDataObj, JS_STR("height"), HeightGetter);
    Nan::SetAccessor(imageDataObj, JS_STR("data"), DataGetter);

    return info.GetReturnValue().Set(imageDataObj);
  } else {
    return Nan::ThrowError("Invalid arguments");
  }
}

NAN_GETTER(ImageData::WidthGetter) {
  Nan::HandleScope scope;

  ImageData *imageData = ObjectWrap::Unwrap<ImageData>(info.This());
  info.GetReturnValue().Set(JS_INT(imageData->GetWidth()));
}

NAN_GETTER(ImageData::HeightGetter) {
  Nan::HandleScope scope;

  ImageData *imageData = ObjectWrap::Unwrap<ImageData>(info.This());
  info.GetReturnValue().Set(JS_INT(imageData->GetHeight()));
}

NAN_GETTER(ImageData::DataGetter) {
  Nan::HandleScope scope;

  ImageData *imageData = ObjectWrap::Unwrap<ImageData>(info.This());
  if (imageData->dataArray.IsEmpty()) {
    SkPixmap pixmap;
    bool ok = imageData->bitmap.peekPixels(&pixmap);

    if (ok) {
      unsigned int width = imageData->GetWidth();
      unsigned int height = imageData->GetHeight();
      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), (void *)pixmap.addr(), width * height * 4); // XXX link lifetime

      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      imageData->dataArray.Reset(uint8ClampedArray);
    } else {
      return info.GetReturnValue().Set(Nan::Null());
    }
  }

  info.GetReturnValue().Set(Nan::New(imageData->dataArray));
}

ImageData::ImageData(unsigned int width, unsigned int height) {
  SkImageInfo info = SkImageInfo::Make(width, height, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
  unsigned char *address = (unsigned char *)malloc(width * height * 4);
  SkPixmap pixmap(info, address, width * 4);
  bitmap.installPixels(pixmap);
}
ImageData::~ImageData () {}

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
  return imageData->getWidth();
}

unsigned int ImageData::GetHeight() {
  return imageData->getHeight();
}

unsigned int ImageData::GetNumChannels() {
  return imageData->getNumChannels();
}

unsigned char *ImageData::GetData() {
  return imageData->getData();
}

void ImageData::Set(canvas::ImageData *imageData) {
  if (this->imageData != nullptr) {
    delete this->imageData;
  }

  this->imageData = imageData;
}

NAN_METHOD(ImageData::New) {
  Nan::HandleScope scope;

  Local<Object> imageDataObj = info.This();

  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    unsigned int width = info[0]->Uint32Value();
    unsigned int height = info[1]->Uint32Value();
    ImageData *imageData = new ImageData(width, height);
    imageData->Wrap(imageDataObj);
  } else {
    ImageData *imageData = new ImageData();
    imageData->Wrap(imageDataObj);
  }

  Nan::SetAccessor(imageDataObj, JS_STR("width"), WidthGetter);
  Nan::SetAccessor(imageDataObj, JS_STR("height"), HeightGetter);
  Nan::SetAccessor(imageDataObj, JS_STR("data"), DataGetter);

  info.GetReturnValue().Set(imageDataObj);
}

NAN_GETTER(ImageData::WidthGetter) {
  Nan::HandleScope scope;

  ImageData *imageData = ObjectWrap::Unwrap<ImageData>(info.This());
  if (imageData->imageData != nullptr) {
    info.GetReturnValue().Set(JS_INT(imageData->GetWidth()));
  } else {
    info.GetReturnValue().Set(JS_INT(0));
  }
}

NAN_GETTER(ImageData::HeightGetter) {
  Nan::HandleScope scope;

  ImageData *imageData = ObjectWrap::Unwrap<ImageData>(info.This());
  if (imageData->imageData != nullptr) {
    info.GetReturnValue().Set(JS_INT(imageData->GetHeight()));
  } else {
    info.GetReturnValue().Set(JS_INT(0));
  }
}

NAN_GETTER(ImageData::DataGetter) {
  Nan::HandleScope scope;

  ImageData *imageData = ObjectWrap::Unwrap<ImageData>(info.This());
  if (imageData->imageData != nullptr) {
    if (imageData->dataArray.IsEmpty()) {
      unsigned int width = imageData->GetWidth();
      unsigned int height = imageData->GetHeight();
      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), imageData->GetData(), width * height * 4);

      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      imageData->dataArray.Reset(uint8ClampedArray);
    }

    info.GetReturnValue().Set(Nan::New(imageData->dataArray));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

ImageData::ImageData() : imageData(nullptr) {}
ImageData::ImageData(unsigned int width, unsigned int height) : imageData(new canvas::ImageData(width, height, 4)) {}
ImageData::~ImageData () {
  if (imageData != nullptr) {
    delete imageData;
  }
}

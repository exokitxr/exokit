#include <canvascontext/include/imageData-context.h>

using namespace v8;
using namespace node;
// using namespace std;

Handle<Object> ImageData::Initialize(Isolate *isolate) {
  v8::EscapableHandleScope scope(isolate);

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("ImageData"));

  // prototype
  // Nan::SetPrototypeMethod(ctor, "save",save);// NODE_SET_PROTOTYPE_METHOD(ctor, "save", save);
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetAccessor(proto,JS_STR("width"), WidthGetter);
  Nan::SetAccessor(proto,JS_STR("height"), HeightGetter);
  Nan::SetAccessor(proto,JS_STR("data"), DataGetter);
  // Nan::SetAccessor(proto,JS_STR("src"), SrcGetter, SrcSetter);
  // Nan::Set(target, JS_STR("Image"), ctor->GetFunction());

  // constructor_template.Reset(Isolate::GetCurrent(), ctor->GetFunction());

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

NAN_METHOD(ImageData::New) {
  Nan::HandleScope scope;

  unsigned int width = info[0]->Uint32Value();
  unsigned int height = info[1]->Uint32Value();
  ImageData *imageData = new ImageData(width, height);
  imageData->Wrap(info.This());
  // registerImage(image);
  info.GetReturnValue().Set(info.This());
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
    unsigned int width = imageData->GetWidth();
    unsigned int height = imageData->GetHeight();
    Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), imageData->GetData(), width * height * 4);

    Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
    imageData->dataArray.Reset(Isolate::GetCurrent(), uint8ClampedArray);
  }

  info.GetReturnValue().Set(Nan::New(imageData->dataArray));
}

ImageData::ImageData(unsigned int width, unsigned int height) {
  imageData = new canvas::ImageData(width, height, 4);
}
ImageData::~ImageData () {
  delete imageData;
}

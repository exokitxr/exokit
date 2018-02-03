#include "image.h"

using namespace v8;
using namespace node;
// using namespace std;

// Persistent<Function> Image::constructor_template;
canvas::ContextFactory *Image::canvasContextFactory;

Handle<Object> Image::Initialize(Isolate *isolate, canvas::ContextFactory *canvasContextFactory) {
  v8::EscapableHandleScope scope(isolate);

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Image"));

  // prototype
  // Nan::SetPrototypeMethod(ctor, "save",save);// NODE_SET_PROTOTYPE_METHOD(ctor, "save", save);
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetAccessor(proto,JS_STR("width"), WidthGetter);
  Nan::SetAccessor(proto,JS_STR("height"), HeightGetter);
  Nan::SetAccessor(proto,JS_STR("data"), DataGetter);
  Nan::SetMethod(proto,"load", LoadMethod);
  // Nan::SetAccessor(proto,JS_STR("src"), SrcGetter, SrcSetter);
  // Nan::Set(target, JS_STR("Image"), ctor->GetFunction());

  // constructor_template.Reset(Isolate::GetCurrent(), ctor->GetFunction());

  Image::canvasContextFactory = canvasContextFactory;

  return scope.Escape(ctor->GetFunction());
}

unsigned int Image::GetWidth() {
  return image->getData().getWidth();
}

unsigned int Image::GetHeight() {
  return image->getData().getHeight();
}

unsigned int Image::GetNumChannels() {
  return image->getData().getNumChannels();
}

unsigned char *Image::GetData() {
  return image->getData().getData();
}

bool Image::Load(const unsigned char *buffer, size_t size) {
  return image->decode(buffer, size);
}

NAN_METHOD(Image::New) {
  Nan::HandleScope scope;

  Image *image = new Image();
  image->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
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
  unsigned int width = image->GetWidth();
  unsigned int height = image->GetHeight();
  Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), image->GetData(), width * height * 4);
  Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());

  info.GetReturnValue().Set(uint8ClampedArray);
}

NAN_METHOD(Image::LoadMethod) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(info.This());
  Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(info[0]);
  v8::ArrayBuffer::Contents contents = arrayBuffer->GetContents();
  unsigned char *data = (unsigned char *)contents.Data();
  size_t size = contents.ByteLength();

  bool result = image->Load(data, size);

  info.GetReturnValue().Set(JS_BOOL(result));
}

Image::Image () {
  image = Image::canvasContextFactory->createImage().release();
}
Image::~Image () {
  delete image;
}

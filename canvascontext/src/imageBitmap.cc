#include "imageBitmap.h"

using namespace v8;
using namespace node;
// using namespace std;

Handle<Object> ImageBitmap::Initialize(Isolate *isolate) {
  v8::EscapableHandleScope scope(isolate);

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("ImageBitmap"));

  // prototype
  // Nan::SetPrototypeMethod(ctor, "save",save);// NODE_SET_PROTOTYPE_METHOD(ctor, "save", save);
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetAccessor(proto,JS_STR("width"), WidthGetter);
  Nan::SetAccessor(proto,JS_STR("height"), HeightGetter);
  Nan::SetAccessor(proto,JS_STR("data"), DataGetter);

  Local<Function> ctorFn = ctor->GetFunction();

  Local<Function> createImageBitmapFn = Nan::New<Function>(CreateImageBitmap);
  createImageBitmapFn->Set(JS_STR("ImageBitmap"), ctorFn);

  ctorFn->Set(JS_STR("createImageBitmap"), createImageBitmapFn);

  // Nan::SetAccessor(proto,JS_STR("src"), SrcGetter, SrcSetter);
  // Nan::Set(target, JS_STR("Image"), ctor->GetFunction());

  // constructor_template.Reset(Isolate::GetCurrent(), ctor->GetFunction());

  return scope.Escape(ctorFn);
}

unsigned int ImageBitmap::GetWidth() {
  return imageData->getWidth();
}

unsigned int ImageBitmap::GetHeight() {
  return imageData->getHeight();
}

unsigned int ImageBitmap::GetNumChannels() {
  return imageData->getNumChannels();
}

unsigned char *ImageBitmap::GetData() {
  return imageData->getData();
}

NAN_METHOD(ImageBitmap::New) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(Local<Object>::Cast(info[0]));
  ImageBitmap *imageBitmap = new ImageBitmap(image);
  imageBitmap->Wrap(info.This());
  // registerImage(image);
  info.GetReturnValue().Set(info.This());
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
  unsigned int width = imageBitmap->GetWidth();
  unsigned int height = imageBitmap->GetHeight();
  Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), imageBitmap->GetData(), width * height * 4);
  Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());

  info.GetReturnValue().Set(uint8ClampedArray);
}

NAN_METHOD(ImageBitmap::CreateImageBitmap) {
  Nan::HandleScope scope;

  Local<String> typeof = info.Callee()->TypeOf(Isolate::GetCurrent());
  String::Utf8Value typeofString(typeof);

  Local<Function> imageBitmapConstructor = Local<Function>::Cast(info.Callee()->Get(JS_STR("ImageBitmap")));
  Local<String> typeof2 = imageBitmapConstructor->TypeOf(Isolate::GetCurrent());
  String::Utf8Value typeofString2(typeof);
  Local<Value> argv[] = {
    info[0],
  };
  Local<Object> imageBitmapObj = imageBitmapConstructor->NewInstance(sizeof(argv) / sizeof(argv[0]), argv);

  info.GetReturnValue().Set(imageBitmapObj);
}

ImageBitmap::ImageBitmap(Image *image) {
  imageData = new canvas::ImageData(image->image->getData());
}
ImageBitmap::~ImageBitmap () {
  delete imageData;
}

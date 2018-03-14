#include <canvascontext/include/canvas-gradient.h>

using namespace v8;
using namespace node;
// using namespace std;

Handle<Object> CanvasGradient::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("CanvasGradient"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto, "addColorStop", AddColorStop);

  return scope.Escape(ctor->GetFunction());
}

CanvasGradient::CanvasGradient(float x0, float y0, float x1, float y1) : type(CanvasGradient::LinearType), x0(x0), y0(y0), x1(x1), y1(y1) {}

CanvasGradient::CanvasGradient(float x0, float y0, float r0, float x1, float y1, float r1) : type(CanvasGradient::RadialType), x0(x0), y0(y0), r0(r0), x1(x1), y1(y1), r1(r1) {}

CanvasGradient::~CanvasGradient () {}

void CanvasGradient::AddColorStop(float offset, SkColor color) {
  positions.push_back(offset);
  colors.push_back(color);
}

NAN_METHOD(CanvasGradient::New) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()) {
    float x0 = info[0]->NumberValue();
    float y0 = info[1]->NumberValue();
    float x1 = info[2]->NumberValue();
    float y1 = info[3]->NumberValue();

    CanvasGradient *canvasGradient = new CanvasGradient(x0, y0, x1, y1);
    canvasGradient->Wrap(info.This());
    // registerImage(image);
    info.GetReturnValue().Set(info.This());
  } else if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber() && info[4]->IsNumber() && info[5]->IsNumber()) {
    float x0 = info[0]->NumberValue();
    float y0 = info[1]->NumberValue();
    float r0 = info[2]->NumberValue();
    float x1 = info[3]->NumberValue();
    float y1 = info[4]->NumberValue();
    float r1 = info[5]->NumberValue();

    CanvasGradient *canvasGradient = new CanvasGradient(x0, y0, r0, x1, y1, r1);
    canvasGradient->Wrap(info.This());
    // registerImage(image);
    info.GetReturnValue().Set(info.This());
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(CanvasGradient::AddColorStop) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsString()) {
    CanvasGradient *canvasGradient = ObjectWrap::Unwrap<CanvasGradient>(info.This());
    
    float offset = info[0]->NumberValue();
    v8::String::Utf8Value colorUtf8Value(Local<String>::Cast(info[1]));
    canvas::web_color webColor = canvas::web_color::from_string(*colorUtf8Value);

    canvasGradient->AddColorStop(offset, ((uint32_t)webColor.a << (8 * 3)) | ((uint32_t)webColor.r << (8 * 2)) | ((uint32_t)webColor.g << (8 * 1)) | ((uint32_t)webColor.b << (8 * 0)));
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

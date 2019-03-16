#include <canvascontext/include/canvas-gradient.h>
#include "../../helpers.h"

using namespace v8;
using namespace node;
// using namespace std;

Local<Object> CanvasGradient::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("CanvasGradient"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto, "addColorStop", AddColorStop);

  return scope.Escape(JS_FUNC(ctor));
}

CanvasGradient::CanvasGradient(float x0, float y0, float x1, float y1) : type(CanvasGradient::LinearType), x0(x0), y0(y0), x1(x1), y1(y1) {}

CanvasGradient::CanvasGradient(float x0, float y0, float r0, float x1, float y1, float r1) : type(CanvasGradient::RadialType), x0(x0), y0(y0), r0(r0), x1(x1), y1(y1), r1(r1) {}

CanvasGradient::~CanvasGradient () {}

sk_sp<SkShader> CanvasGradient::getShader() const {
  if (type == CanvasGradient::LinearType) {
    SkPoint points[] = {
      SkPoint::Make(x0, y0),
      SkPoint::Make(x1, y1),
    };
    return SkGradientShader::MakeLinear(points, colors.data(), positions.data(), colors.size(), SkShader::kClamp_TileMode, 0, nullptr);
  } else if (type == CanvasGradient::RadialType) {
    return SkGradientShader::MakeRadial(SkPoint::Make(x1, y1), r1, colors.data(), positions.data(), colors.size(), SkShader::kClamp_TileMode, 0, nullptr);
  } else {
    return nullptr;
  }
}

void CanvasGradient::AddColorStop(SkScalar offset, SkColor color) {
  positions.push_back(offset);
  colors.push_back(color);
}

NAN_METHOD(CanvasGradient::New) {
  Nan::HandleScope scope;

  if (info.Length() == 4 && info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()) {
    float x0 = JS_NUM(info[0]);
    float y0 = JS_NUM(info[1]);
    float x1 = JS_NUM(info[2]);
    float y1 = JS_NUM(info[3]);

    CanvasGradient *canvasGradient = new CanvasGradient(x0, y0, x1, y1);
    canvasGradient->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else if (info.Length() == 6 && info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber() && info[4]->IsNumber() && info[5]->IsNumber()) {
    float x0 = JS_NUM(info[0]);
    float y0 = JS_NUM(info[1]);
    float r0 = JS_NUM(info[2]);
    float x1 = JS_NUM(info[3]);
    float y1 = JS_NUM(info[4]);
    float r1 = JS_NUM(info[5]);

    CanvasGradient *canvasGradient = new CanvasGradient(x0, y0, r0, x1, y1, r1);
    canvasGradient->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(CanvasGradient::AddColorStop) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsString()) {
    CanvasGradient *canvasGradient = ObjectWrap::Unwrap<CanvasGradient>(info.This());
    
    SkScalar offset = JS_NUM(info[0]);
    Nan::Utf8String colorUtf8Value(Local<String>::Cast(info[1]));
    canvas::web_color webColor = canvas::web_color::from_string(*colorUtf8Value);

    canvasGradient->AddColorStop(offset, ((uint32_t)webColor.a << (8 * 3)) | ((uint32_t)webColor.r << (8 * 2)) | ((uint32_t)webColor.g << (8 * 1)) | ((uint32_t)webColor.b << (8 * 0)));
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

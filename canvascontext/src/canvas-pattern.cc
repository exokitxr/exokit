#include <canvascontext/include/canvas-pattern.h>

using namespace v8;
using namespace node;
// using namespace std;

Handle<Object> CanvasPattern::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("CanvasPattern"));

  // prototype
  // Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  return scope.Escape(ctor->GetFunction());
}

CanvasPattern::CanvasPattern(sk_sp<SkShader> shader) : shader(shader) {}

CanvasPattern::~CanvasPattern () {}

NAN_METHOD(CanvasPattern::New) {
  Nan::HandleScope scope;

  sk_sp<SkImage> image = CanvasRenderingContext2D::getImage(info[0]);
  if (image) {
    std::string repetition;
    if (info[1]->IsString()) {
      v8::String::Utf8Value repetitionUtf8Value(info[1]);
      repetition = std::string(*repetitionUtf8Value, repetitionUtf8Value.length());
    }

    SkShader::TileMode tmx;
    SkShader::TileMode tmy;
    if (repetition == "repeat") {
      tmx = SkShader::kRepeat_TileMode;
      tmy = SkShader::kRepeat_TileMode;
    } else if (repetition == "repeat-x") {
      tmx = SkShader::kRepeat_TileMode;
      tmy = SkShader::kClamp_TileMode;
    } else if (repetition == "repeat-y") {
      tmx = SkShader::kClamp_TileMode;
      tmy = SkShader::kRepeat_TileMode;
    } else if (repetition == "no-repeat") {
      tmx = SkShader::kClamp_TileMode;
      tmy = SkShader::kClamp_TileMode;
    } else {
      tmx = SkShader::kRepeat_TileMode;
      tmy = SkShader::kRepeat_TileMode;
    }

    SkBitmap bitmap;
    image->asLegacyBitmap(&bitmap, SkImage::kRO_LegacyBitmapMode);
    sk_sp<SkShader> shader = SkShader::MakeBitmapShader(bitmap, tmx, tmy);
    
    CanvasPattern *canvasPattern = new CanvasPattern(shader);
    canvasPattern->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

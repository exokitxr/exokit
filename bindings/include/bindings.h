#ifndef _EXOKIT_BINDINGS_H_
#define _EXOKIT_BINDINGS_H_

#include <v8.h>
#include <node.h>
#include <image-context.h>
#include <imageData-context.h>
#include <imageBitmap-context.h>
#include <canvas-context.h>
#include <path2d-context.h>
#include <webgl.h>
#include <AudioContext.h>
#include <Video.h>

class WebGLContext : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  WebGLContext();
  ~WebGLContext();

  static NAN_METHOD(New);
  static NAN_GETTER(OnCallGetter);
  static NAN_SETTER(OnCallSetter);

  Nan::Persistent<Function> oncall;

  template<NAN_METHOD(F)>
  static NAN_METHOD(glOnCallWrap) {
    {
      Nan::HandleScope scope;

      Local<Object> glObj = info.This();
      WebGLContext *gl = ObjectWrap::Unwrap<WebGLContext>(glObj);
      if (!gl->oncall.IsEmpty()) {
        Local<Function> oncallFn = Nan::New(gl->oncall);
        oncallFn->Call(glObj, 0, nullptr);
      }
    }

    F(info);
  }
};

v8::Local<v8::Object> makeGl();
v8::Local<v8::Object> makeImage();
v8::Local<v8::Object> makeImageData();
v8::Local<v8::Object> makeImageBitmap(Local<Value> imageCons);
v8::Local<v8::Object> makeCanvasRenderingContext2D(Local<Value> imageDataCons);
v8::Local<v8::Object> makePath2D();
v8::Local<v8::Object> makeAudio();
v8::Local<v8::Object> makeVideo();

#endif

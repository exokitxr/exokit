#ifndef _EXOKIT_BINDINGS_H_
#define _EXOKIT_BINDINGS_H_

#include <v8.h>
#include <node.h>
#include <image-context.h>
#include <imageData-context.h>
#include <imageBitmap-context.h>
#include <canvas-context.h>
#include <path2d-context.h>
#include <canvas-gradient.h>
#include <glfw.h>
#include <webgl.h>
#include <AudioContext.h>
#include <Video.h>

class WebGLRenderingContext : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  WebGLRenderingContext();
  ~WebGLRenderingContext();

  static NAN_METHOD(New);
  static NAN_METHOD(Destroy);
  static NAN_METHOD(GetWindowHandle);
  static NAN_METHOD(SetWindowHandle);
  static NAN_METHOD(IsDirty);
  static NAN_METHOD(ClearDirty);

  bool live;
  GLFWwindow *windowHandle;
  bool dirty;

  template<NAN_METHOD(F)>
  static NAN_METHOD(glCallWrap) {
    Nan::HandleScope scope;

    Local<Object> glObj = info.This();
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(glObj);
    if (gl->live) {
      if (gl->windowHandle) {
        glfw::SetCurrentWindowContext(gl->windowHandle);
      }

      gl->dirty = true;

      F(info);
    }
  }
};

v8::Local<v8::Object> makeGl();
v8::Local<v8::Object> makeImage();
v8::Local<v8::Object> makeImageData();
v8::Local<v8::Object> makeImageBitmap(Local<Value> imageCons);
v8::Local<v8::Object> makeCanvasRenderingContext2D(Local<Value> imageDataCons, Local<Value> canvasGradientCons);
v8::Local<v8::Object> makePath2D();
v8::Local<v8::Object> makeCanvasGradient();
v8::Local<v8::Object> makeAudio();
v8::Local<v8::Object> makeVideo();

#endif

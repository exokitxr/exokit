#ifndef _BROWSER_ANDROID_H_
#define _BROWSER_ANDROID_H_

#include <jni.h>
#include <android_native_app_glue.h>

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>
#include <windowsystem.h>
#include <webgl.h>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

class Browser : public ObjectWrap {
public:
  Browser(WebGLRenderingContext *gl, int width, int height, const std::string &urlString);
  ~Browser();

  static NAN_METHOD(New);
  static NAN_GETTER(TextureGetter);

  static Local<Object> Initialize(Isolate *isolate);

  GLuint tex;
};

}

#endif

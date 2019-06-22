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

class Browser {
public:
  Browser(GLuint externalTex, GLuint tex, int width, int height, const std::string &urlString);
  ~Browser();

  GLuint externalTex;
  GLuint tex;

  GLuint renderFbo;
  GLuint renderVao;
  GLuint renderProgram;

  GLuint renderVertex;
  GLuint renderFragment;
  GLuint positionLocation;
};

class BrowserWrap : public ObjectWrap {
public:
  BrowserWrap(Browser *browser);
  ~BrowserWrap();

  static NAN_METHOD(New);
  static NAN_GETTER(TextureGetter);

  static Local<Object> Initialize(Isolate *isolate);

  Browser *browser;
};

extern std::vector<std::function<void()>> mainThreadFns;
extern std::vector<std::function<void()>> persistentMainThreadFns;
void RunInMainThread(std::function<void()> fn);
NAN_METHOD(PollEvents);

}

#endif

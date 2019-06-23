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
  Browser(JNIEnv *env, jobject context, GLuint externalTex, GLuint tex, int width, int height, const std::string &urlString);
  ~Browser();

  void KeyDown(int keyCode);
  void KeyUp(int keyCode);
  void KeyPress(int keyCode);
  void MouseDown(int x, int y, int button);
  void MouseUp(int x, int y, int button);
  void Click(int x, int y, int button);
  void MouseMove(int x, int y);
  void MouseWheel(int x, int y, int deltaX, int deltaY);

  JNIEnv *env;
  jobject context;
  jobject exokitWebView;
  // jmethodID runJsFnId;
  jmethodID keyDownFnId;
  jmethodID keyUpFnId;
  jmethodID keyPressFnId;
  jmethodID mouseDownFnId;
  jmethodID mouseUpFnId;
  jmethodID clickFnId;
  jmethodID mouseMoveFnId;
  jmethodID mouseWheelFnId;

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
  static NAN_METHOD(KeyDown);
  static NAN_METHOD(KeyUp);
  static NAN_METHOD(KeyPress);
  static NAN_METHOD(MouseDown);
  static NAN_METHOD(MouseUp);
  static NAN_METHOD(Click);
  static NAN_METHOD(MouseMove);
  static NAN_METHOD(MouseWheel);

  static Local<Object> Initialize(Isolate *isolate);

  Browser *browser;
};

extern std::vector<std::function<void()>> mainThreadFns;
extern std::vector<std::function<void()>> persistentMainThreadFns;
void RunInMainThread(std::function<void()> fn);
void QueueInMainThread(std::function<void()> fn);
NAN_METHOD(PollEvents);

}

#endif

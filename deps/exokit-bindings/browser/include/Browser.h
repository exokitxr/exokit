#ifndef _BROWSER_H_
#define _BROWSER_H_

#include <webgl.h>
#include "web_core.h"

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {
  
extern WebCoreManager g_web_core_manager;
extern bool g_web_core_manager_initialized;
extern std::string g_resources_path;
extern std::string g_locales_path;

class Browser : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  void Update();

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(Update);
  static NAN_METHOD(SetResourcesPath);
  static NAN_METHOD(SetLocalesPath);

  Browser(WebGLRenderingContext *gl, GLuint tex, int width, int height, const std::string &url);
  ~Browser();

protected:
  GLuint tex;
  bool initialized;
  std::weak_ptr<WebCore> web_core;
};

}

#endif

#include <browser.h>

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>

// #include <functional>
#include <iostream>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

Browser::Browser(WebGLRenderingContext *gl, GLuint tex, int width, int height, const std::string &url) : tex(tex), initialized(false) {
  web_core = g_web_core_manager.createBrowser(url, [this, gl](const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) -> void {
    size_t count = 0;
    for (int i = 0; i < width * height * 4; i += 4) {
      if (((const char *)buffer)[i]) {
        count++;
      }
    }
    
    glBindTexture(GL_TEXTURE_2D, this->tex);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

    if (!this->initialized) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

      this->initialized = true;
    }

    for (size_t i = 0; i < dirtyRects.size(); i++) {
      const CefRect &rect = dirtyRects[i];
      
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect.x);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, rect.y);
      glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D)) {
      glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D));
    } else {
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  });
  web_core.lock()->reshape(width, height);
}

Browser::~Browser() {}

Handle<Object> Browser::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Browser"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "update", Update);

  Local<Function> ctorFn = ctor->GetFunction();
  Nan::SetMethod(ctorFn, "setResourcesPath", SetResourcesPath);
  Nan::SetMethod(ctorFn, "setLocalesPath", SetLocalesPath);

  return scope.Escape(ctorFn);
}

NAN_METHOD(Browser::New) {
  // Nan::HandleScope scope;

  if (
    info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("WebGLRenderingContext")) &&
    info[1]->IsObject() && info[1]->ToObject()->Get(JS_STR("id"))->IsNumber() &&
    info[2]->IsNumber() &&
    info[3]->IsNumber() &&
    info[4]->IsString()
  ) {
    if (!g_web_core_manager_initialized) {
      int exit_code = 0;
      std::cout << "initialize web core manager 1" << std::endl;
      const bool success = g_web_core_manager.setUp(&exit_code);
      std::cout << "initialize web core manager 2 " << success << std::endl;
      if (success) {
        g_web_core_manager_initialized = true;
      } else {
        return Nan::ThrowError("Browser::Browser: failed to set up core manager");
      }
    }

    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
    GLuint tex = Local<Object>::Cast(info[1])->Get(JS_STR("id"))->Uint32Value();
    int width = info[2]->Int32Value();
    int height = info[3]->Int32Value();
    String::Utf8Value urlUtf8Value(Local<String>::Cast(info[4]));
    std::string url(*urlUtf8Value, urlUtf8Value.length());

    Browser *browser = new Browser(gl, tex, width, height, url);
    Local<Object> browserObj = info.This();
    browser->Wrap(browserObj);

    return info.GetReturnValue().Set(browserObj);
  } else {
    return Nan::ThrowError("Browser::New: invalid arguments");
  }
}

void Browser::Update() {
  g_web_core_manager.update();
}

NAN_METHOD(Browser::Update) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  // std::cout << "browser update 1" << std::endl;
  browser->Update();
  // std::cout << "browser update 2" << std::endl;
}

NAN_METHOD(Browser::SetResourcesPath) {
  if (info[0]->IsString()) {
    String::Utf8Value utf8Value(Local<String>::Cast(info[0]));
    g_resources_path = std::string(*utf8Value, utf8Value.length());
  } else {
    Nan::ThrowError("Browser::SetResourcesPath: invalid arguments");
  }
}

NAN_METHOD(Browser::SetLocalesPath) {
  if (info[0]->IsString()) {
    String::Utf8Value utf8Value(Local<String>::Cast(info[0]));
    g_locales_path = std::string(*utf8Value, utf8Value.length());
  } else {
    Nan::ThrowError("Browser::SetLocalesPath: invalid arguments");
  }
}

WebCoreManager g_web_core_manager;
bool g_web_core_manager_initialized = false;
std::string g_resources_path;
std::string g_locales_path;

}

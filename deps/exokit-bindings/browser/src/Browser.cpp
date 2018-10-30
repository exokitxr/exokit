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

bool initializeCef() {
  CefMainArgs args;
  
	CefSettings settings;
  // settings.log_severity = LOGSEVERITY_VERBOSE;
  // CefString(&settings.resources_dir_path) = resourcesPath;
  // CefString(&settings.locales_dir_path) = localesPath;
  settings.no_sandbox = true;
  
  SimpleApp *app = new SimpleApp();
  
	return CefInitialize(args, settings, app, nullptr);
}

Browser::Browser(WebGLRenderingContext *gl, GLuint tex, int width, int height, const std::string &url) : tex(tex), initialized(false) {
  web_core.reset(new WebCore(url, [this, gl](const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) -> void {
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
  }));
  web_core->reshape(width, height);
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
    if (!cefInitialized) {
      // std::cout << "initialize web core manager 1" << std::endl;
      const bool success = initializeCef();
      // std::cout << "initialize web core manager 2 " << success << std::endl;
      if (success) {
        cefInitialized = true;
      } else {
        return Nan::ThrowError("Browser::Browser: failed to initialize CEF");
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
  CefDoMessageLoopWork();
}

NAN_METHOD(Browser::Update) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  // std::cout << "browser update 1" << std::endl;
  browser->Update();
  // std::cout << "browser update 2" << std::endl;
}

bool cefInitialized = false;

}

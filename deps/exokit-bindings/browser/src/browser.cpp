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

// helpers

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

// SimpleApp

SimpleApp::SimpleApp() {}

void SimpleApp::OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line) {
  command_line->AppendSwitch(CefString("single-process"));
  // command_line->AppendSwitch(CefString("no-proxy-server"));
  command_line->AppendSwitch(CefString("winhttp-proxy-resolver"));
  command_line->AppendSwitch(CefString("no-sandbox"));
}

void SimpleApp::OnContextInitialized() {
  // CEF_REQUIRE_UI_THREAD();
  
  // std::cout << "SimpleApp::OnContextInitialized" << std::endl;
}

// LoadHandler

LoadHandler::LoadHandler(std::function<void()> onLoadStart, std::function<void()> onLoadEnd, std::function<void()> onLoadError) : onLoadStart(onLoadStart), onLoadEnd(onLoadEnd), onLoadError(onLoadError) {}

LoadHandler::~LoadHandler() {}

void LoadHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) {
  onLoadStart();
}

void LoadHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) {
  onLoadEnd();
}

void LoadHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString &failedUrl) {
  onLoadError();
}

// DisplayHandler

DisplayHandler::DisplayHandler(std::function<void(const std::string &, const std::string &, int)> onConsole, std::function<void(const std::string &)> onMessage) : onConsole(onConsole), onMessage(onMessage) {}

DisplayHandler::~DisplayHandler() {}

const std::string postMessageConsolePrefix("<postMessage>");
bool DisplayHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level, const CefString &message, const CefString &source, int line) {
  std::string m = message.ToString();
  
  if (!m.compare(0, postMessageConsolePrefix.size(), postMessageConsolePrefix)) {
    onMessage(m.substr(postMessageConsolePrefix.size()));
  } else {
    onConsole(m, source.length() > 0 ? source.ToString() : std::string("<unknown>"), line);
  }
  
  return true;
}

// RenderHandler

RenderHandler::RenderHandler(OnPaintFn onPaint) : onPaint(onPaint), width(1280), height(1024) {}

RenderHandler::~RenderHandler() {}

void RenderHandler::resize(int w, int h) {
	width = w;
	height = h;
}

bool RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) {
	rect = CefRect(0, 0, width, height);
	return true;
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) {
  onPaint(dirtyRects, buffer, width, height);
}

// BrowserClient

BrowserClient::BrowserClient(LoadHandler *loadHandler, DisplayHandler *displayHandler, RenderHandler *renderHandler) : m_loadHandler(loadHandler), m_displayHandler(displayHandler), m_renderHandler(renderHandler) {}

BrowserClient::~BrowserClient() {}

// Browser

Browser::Browser(WebGLRenderingContext *gl, int width, int height, const std::string &url) : tex(0), initialized(false) {
  ensureCurrentGlWindow(gl);
  
  glGenTextures(1, &tex);

  QueueOnBrowserThread([&]() -> void {
    load_handler_.reset(
      new LoadHandler(
        [this]() -> void {
          browser_->GetMainFrame()->ExecuteJavaScript(CefString("window.postMessage = m => {console.log('<postMessage>' + JSON.stringify(m));};"), CefString("<bootstrap>"), 1);
          
          RunOnMainThread([&]() -> void {
            Nan::HandleScope scope;
            
            if (!this->onloadstart.IsEmpty()) {
              Local<Function> onloadstart = Nan::New(this->onloadstart);
              onloadstart->Call(Nan::Null(), 0, nullptr);
            }
          });
        },
        [this]() -> void {
          RunOnMainThread([&]() -> void {
            Nan::HandleScope scope;
            
            if (!this->onloadend.IsEmpty()) {
              Local<Function> onloadend = Nan::New(this->onloadend);
              onloadend->Call(Nan::Null(), 0, nullptr);
            }
          });
        },
        [this]() -> void {
          RunOnMainThread([&]() -> void {
            Nan::HandleScope scope;
            
            if (!this->onloaderror.IsEmpty()) {
              Local<Function> onloaderror = Nan::New(this->onloaderror);
              onloaderror->Call(Nan::Null(), 0, nullptr);
            }
          });
        }
      )
    );
    
    display_handler_.reset(
      new DisplayHandler(
        [this](const std::string &jsString, const std::string &scriptUrl, int startLine) -> void {
          RunOnMainThread([&]() -> void {
            Nan::HandleScope scope;
            
            if (!this->onconsole.IsEmpty()) {
              Local<Function> onconsole = Nan::New(this->onconsole);
              Local<Value> argv[] = {
                JS_STR(jsString),
                JS_STR(scriptUrl),
                JS_INT(startLine),
              };
              onconsole->Call(Nan::Null(), sizeof(argv)/sizeof(argv[0]), argv);
            }
          });
        },
        [this](const std::string &m) -> void {
          RunOnMainThread([&]() -> void {
            Nan::HandleScope scope;
            
            if (!this->onmessage.IsEmpty()) {
              Local<Function> onmessage = Nan::New(this->onmessage);
              Local<Value> argv[] = {
                JS_STR(m),
              };
              onmessage->Call(Nan::Null(), sizeof(argv)/sizeof(argv[0]), argv);
            }
          });
        }
      )
    );
    
    render_handler_.reset(
      new RenderHandler(
        [this, gl](const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) -> void {
          RunOnMainThread([&]() -> void {
            ensureCurrentGlWindow(gl);
            
            glBindTexture(GL_TEXTURE_2D, this->tex);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, width); // XXX save/restore these

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
        }
      )
    );
    render_handler_->resize(128, 128);

    CefWindowInfo window_info;
    window_info.SetAsWindowless(nullptr);
    CefBrowserSettings browserSettings;
    // browserSettings.windowless_frame_rate = 60; // 30 is default
    client_.reset(new BrowserClient(load_handler_.get(), display_handler_.get(), render_handler_.get()));
    
    browser_ = CefBrowserHost::CreateBrowserSync(window_info, client_.get(), url, browserSettings, nullptr);
    
    this->reshape(width, height);
    
    uv_sem_post(&constructSem);
  });
  
  uv_sem_wait(&constructSem);
}

Browser::~Browser() {}

Handle<Object> Browser::Initialize(Isolate *isolate) {
  uv_async_init(uv_default_loop(), &mainThreadAsync, MainThreadAsync);
  uv_sem_init(&constructSem, 0);
  uv_sem_init(&mainThreadSem, 0);
  uv_sem_init(&browserThreadSem, 0);
  
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Browser"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetAccessor(proto, JS_STR("onloadstart"), OnLoadStartGetter, OnLoadStartSetter);
  Nan::SetAccessor(proto, JS_STR("onloadend"), OnLoadEndGetter, OnLoadEndSetter);
  Nan::SetAccessor(proto, JS_STR("onloaderror"), OnLoadErrorGetter, OnLoadErrorSetter);
  Nan::SetAccessor(proto, JS_STR("onconsole"), OnConsoleGetter, OnConsoleSetter);
  Nan::SetAccessor(proto, JS_STR("onmessage"), OnMessageGetter, OnMessageSetter);
  Nan::SetMethod(proto, "back", Back);
  Nan::SetMethod(proto, "forward", Forward);
  Nan::SetMethod(proto, "reload", Reload);
  Nan::SetMethod(proto, "sendMouseMove", SendMouseMove);
  Nan::SetMethod(proto, "sendMouseDown", SendMouseDown);
  Nan::SetMethod(proto, "sendMouseUp", SendMouseUp);
  Nan::SetMethod(proto, "sendMouseWheel", SendMouseWheel);
  Nan::SetMethod(proto, "sendKeyDown", SendKeyDown);
  Nan::SetMethod(proto, "sendKeyUp", SendKeyUp);
  Nan::SetMethod(proto, "sendKeyPress", SendKeyPress);
  Nan::SetMethod(proto, "runJs", RunJs);
  Nan::SetMethod(proto, "postMessage", PostMessage);

  Local<Function> ctorFn = ctor->GetFunction();
  Nan::SetMethod(ctorFn, "updateAll", UpdateAll);

  return scope.Escape(ctorFn);
}

NAN_METHOD(Browser::New) {
  if (
    info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("WebGLRenderingContext")) &&
    info[1]->IsNumber() &&
    info[2]->IsNumber() &&
    info[3]->IsString()
  ) {
    if (!cefInitialized) {
      browserThread = std::thread([]() -> void {
        // std::cout << "initialize web core manager 1" << std::endl;
        const bool success = initializeCef();
        // std::cout << "initialize web core manager 2 " << success << std::endl;
        if (success) {          
          for (;;) {
            uv_sem_wait(&browserThreadSem);
            
            std::function<void()> fn;
            {
              std::lock_guard<std::mutex> lock(browserThreadFnMutex);

              fn = browserThreadFns.front();
              browserThreadFns.pop_front();
            }
            
            fn();
          }
        } else {
          std::cerr << "Browser::Browser: failed to initialize CEF" << std::endl;
        }
      });
      cefInitialized = true;
    }

    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
    int width = info[1]->Int32Value();
    int height = info[2]->Int32Value();
    String::Utf8Value urlUtf8Value(Local<String>::Cast(info[3]));
    std::string url(*urlUtf8Value, urlUtf8Value.length());

    Browser *browser = new Browser(gl, width, height, url);
    Local<Object> browserObj = info.This();
    browser->Wrap(browserObj);
    
    Nan::SetAccessor(browserObj, JS_STR("texture"), TextureGetter);

    return info.GetReturnValue().Set(browserObj);
  } else {
    return Nan::ThrowError("Browser::New: invalid arguments");
  }
}

void Browser::reshape(int w, int h) {
	render_handler_->resize(w, h);
	browser_->GetHost()->WasResized();
}

NAN_METHOD(Browser::UpdateAll) {
  if (cefInitialized) {
    QueueOnBrowserThread([]() -> void {
      // std::cout << "browser update 1" << std::endl;
      CefDoMessageLoopWork();
      // std::cout << "browser update 2" << std::endl;
    });
  }
}

NAN_GETTER(Browser::OnLoadStartGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  Local<Function> onloadstart = Nan::New(browser->onloadstart);
  info.GetReturnValue().Set(onloadstart);
}
NAN_SETTER(Browser::OnLoadStartSetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  if (value->IsFunction()) {
    Local<Function> onloadstart = Local<Function>::Cast(value);
    browser->onloadstart.Reset(onloadstart);
  } else {
    browser->onloadstart.Reset();
  }
}
NAN_GETTER(Browser::OnLoadEndGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  Local<Function> onloadend = Nan::New(browser->onloadend);
  info.GetReturnValue().Set(onloadend);
}
NAN_SETTER(Browser::OnLoadEndSetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  if (value->IsFunction()) {
    Local<Function> onloadend = Local<Function>::Cast(value);
    browser->onloadend.Reset(onloadend);
  } else {
    browser->onloadend.Reset();
  }
}
NAN_GETTER(Browser::OnLoadErrorGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  Local<Function> onloaderror = Nan::New(browser->onloaderror);
  info.GetReturnValue().Set(onloaderror);
}
NAN_SETTER(Browser::OnLoadErrorSetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  if (value->IsFunction()) {
    Local<Function> onloaderror = Local<Function>::Cast(value);
    browser->onloaderror.Reset(onloaderror);
  } else {
    browser->onloaderror.Reset();
  }
}

NAN_GETTER(Browser::OnConsoleGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  Local<Function> onconsole = Nan::New(browser->onconsole);
  info.GetReturnValue().Set(onconsole);
}
NAN_SETTER(Browser::OnConsoleSetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  if (value->IsFunction()) {
    Local<Function> onconsole = Local<Function>::Cast(value);
    browser->onconsole.Reset(onconsole);
  } else {
    browser->onconsole.Reset();
  }
}

NAN_GETTER(Browser::OnMessageGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  Local<Function> onmessage = Nan::New(browser->onmessage);
  info.GetReturnValue().Set(onmessage);
}
NAN_SETTER(Browser::OnMessageSetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  if (value->IsFunction()) {
    Local<Function> onmessage = Local<Function>::Cast(value);
    browser->onmessage.Reset(onmessage);
  } else {
    browser->onmessage.Reset();
  }
}

NAN_METHOD(Browser::Back) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  CefBrowser *cefBrowser = browser->browser_.get();
  
  QueueOnBrowserThread([cefBrowser]() -> void {
    cefBrowser->GoBack();
  });
}

NAN_METHOD(Browser::Forward) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  CefBrowser *cefBrowser = browser->browser_.get();
  
  QueueOnBrowserThread([cefBrowser]() -> void {
    cefBrowser->GoForward();
  });
}

NAN_METHOD(Browser::Reload) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  CefBrowser *cefBrowser = browser->browser_.get();
  
  QueueOnBrowserThread([cefBrowser]() -> void {
    cefBrowser->Reload();
  });
}

NAN_METHOD(Browser::SendMouseMove) {
  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int x = info[0]->Int32Value();
    int y = info[1]->Int32Value();
    CefBrowser *cefBrowser = browser->browser_.get();
    
    QueueOnBrowserThread([x, y, cefBrowser]() -> void {
      CefMouseEvent evt;
      evt.x = x;
      evt.y = y;

      cefBrowser->GetHost()->SendMouseMoveEvent(evt, false);
    });
  } else {
    return Nan::ThrowError("Browser::SendMouseMove: invalid arguments");
  }
}

NAN_METHOD(Browser::SendMouseDown) {
  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int x = info[0]->Int32Value();
    int y = info[1]->Int32Value();
    CefBrowser *cefBrowser = browser->browser_.get();
    
    QueueOnBrowserThread([x, y, cefBrowser]() -> void {
      CefMouseEvent evt;
      evt.x = x;
      evt.y = y;

      cefBrowser->GetHost()->SendMouseClickEvent(evt, CefBrowserHost::MouseButtonType::MBT_LEFT, false, 1);
    });
  } else {
    return Nan::ThrowError("Browser::SendMouseDown: invalid arguments");
  }
}

NAN_METHOD(Browser::SendMouseUp) {
  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int x = info[0]->Int32Value();
    int y = info[1]->Int32Value();
    CefBrowser *cefBrowser = browser->browser_.get();
    
    QueueOnBrowserThread([x, y, cefBrowser]() -> void {
      CefMouseEvent evt;
      evt.x = x;
      evt.y = y;

      cefBrowser->GetHost()->SendMouseClickEvent(evt, CefBrowserHost::MouseButtonType::MBT_LEFT, true, 1);
    });
  } else {
    return Nan::ThrowError("Browser::SendMouseUp: invalid arguments");
  }
}

NAN_METHOD(Browser::SendMouseWheel) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int x = info[0]->Int32Value();
    int y = info[1]->Int32Value();
    int deltaX = info[2]->Int32Value();
    int deltaY = info[3]->Int32Value();
    CefBrowser *cefBrowser = browser->browser_.get();
    
    QueueOnBrowserThread([x, y, deltaX, deltaY, cefBrowser]() -> void {
      CefMouseEvent evt;
      evt.x = x;
      evt.y = y;

      cefBrowser->GetHost()->SendMouseWheelEvent(evt, deltaX, deltaY);
    });
  } else {
    return Nan::ThrowError("Browser::SendMouseUp: invalid arguments");
  }
}

NAN_METHOD(Browser::SendKeyDown) {
  if (info[0]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int key = info[0]->Int32Value();
    CefBrowser *cefBrowser = browser->browser_.get();
    
    QueueOnBrowserThread([key, cefBrowser]() -> void {
      CefKeyEvent evt;
      evt.type = KEYEVENT_RAWKEYDOWN;
      evt.character = key;
      evt.native_key_code = key;
      evt.windows_key_code = key;

      cefBrowser->GetHost()->SendKeyEvent(evt);
    });
  } else {
    return Nan::ThrowError("Browser::SendKeyDown: invalid arguments");
  }
}

NAN_METHOD(Browser::SendKeyUp) {
  if (info[0]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int key = info[0]->Int32Value();
    CefBrowser *cefBrowser = browser->browser_.get();
    
    QueueOnBrowserThread([key, cefBrowser]() -> void {
      CefKeyEvent evt;
      evt.type = KEYEVENT_KEYUP;
      evt.character = key;
      evt.native_key_code = key;
      evt.windows_key_code = key;

      cefBrowser->GetHost()->SendKeyEvent(evt);
    });

  } else {
    return Nan::ThrowError("Browser::SendKeyUp: invalid arguments");
  }
}

NAN_METHOD(Browser::SendKeyPress) {
  if (info[0]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int key = info[0]->Uint32Value();
    CefBrowser *cefBrowser = browser->browser_.get();
    
    QueueOnBrowserThread([key, cefBrowser]() -> void {
      CefKeyEvent evt;
      evt.type = KEYEVENT_CHAR;
      evt.character = key;
      evt.native_key_code = key;
      evt.windows_key_code = key;

      cefBrowser->GetHost()->SendKeyEvent(evt);
    });

  } else {
    return Nan::ThrowError("Browser::SendKeyPress: invalid arguments");
  }
}

NAN_METHOD(Browser::RunJs) {
  if (info[0]->IsString() && info[1]->IsString() && info[2]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    
    String::Utf8Value jsStringValue(Local<String>::Cast(info[0]));
    string jsString(*jsStringValue, jsStringValue.length());
    
    String::Utf8Value scriptUrlValue(Local<String>::Cast(info[1]));
    string scriptUrl(*scriptUrlValue, scriptUrlValue.length());
    
    int startLine = info[2]->Int32Value();
    
    CefBrowser *cefBrowser = browser->browser_.get();
    
    QueueOnBrowserThread([jsString, scriptUrl, startLine, cefBrowser]() -> void {
      cefBrowser->GetMainFrame()->ExecuteJavaScript(CefString(jsString), CefString(scriptUrl), startLine);
      
      uv_sem_post(&constructSem);
    });
    
    uv_sem_wait(&constructSem);
  } else {
    return Nan::ThrowError("Browser::RunJs: invalid arguments");
  }
}

const char *postMessagePrefix = "window.dispatchEvent(new MessageEvent('message', {data: ";
const char *postMessageSuffix = "}));";
NAN_METHOD(Browser::PostMessage) {
  if (info[0]->IsString()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    
    String::Utf8Value messageJsonValue(Local<String>::Cast(info[0]));
    string messageJson(*messageJsonValue, messageJsonValue.length());
    
    CefBrowser *cefBrowser = browser->browser_.get();
    
    QueueOnBrowserThread([messageJson, cefBrowser]() -> void {
      std::string jsString(postMessagePrefix);
      jsString += messageJson;
      jsString += postMessageSuffix;
      cefBrowser->GetMainFrame()->ExecuteJavaScript(CefString(jsString), CefString("<postMessage>"), 1);
    });
  } else {
    return Nan::ThrowError("Browser::RunJs: invalid arguments");
  }
}

NAN_GETTER(Browser::TextureGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  Local<Object> textureObj = Nan::New<Object>();
  textureObj->Set(JS_STR("id"), JS_INT(browser->tex));
  info.GetReturnValue().Set(textureObj);
}

// helpers

void QueueOnBrowserThread(std::function<void()> fn) {
  {
    std::lock_guard<std::mutex> lock(browserThreadFnMutex);
    browserThreadFns.push_front(fn); // push_front for fifo
  }
  
  uv_sem_post(&browserThreadSem);
}

void RunOnMainThread(std::function<void()> fn) {
  {
    std::lock_guard<std::mutex> lock(mainThreadFnMutex);
    mainThreadFns.push_back(fn);
  }

  uv_async_send(&mainThreadAsync);
  uv_sem_wait(&mainThreadSem);
}

void MainThreadAsync(uv_async_t *handle) {
  {
    std::lock_guard<std::mutex> lock(mainThreadFnMutex);
    
    for (size_t i = 0; i < mainThreadFns.size(); i++) {
      mainThreadFns[i]();
    }
    mainThreadFns.clear();
  }

  uv_sem_post(&mainThreadSem);
}

// variables

bool cefInitialized = false;
std::thread browserThread;

uv_sem_t constructSem;
uv_sem_t mainThreadSem;
uv_sem_t browserThreadSem;

std::mutex browserThreadFnMutex;
std::deque<std::function<void()>> browserThreadFns;

uv_async_t mainThreadAsync;
std::mutex mainThreadFnMutex;
std::deque<std::function<void()>> mainThreadFns;

}

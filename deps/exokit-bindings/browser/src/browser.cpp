#include <browser.h>

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <iostream>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

// Browser

Browser::Browser(WebGLRenderingContext *gl, int width, int height, const std::string &url) : gl(gl), tex(0), textureWidth(0), textureHeight(0) {
  windowsystem::SetCurrentWindowContext(gl->windowHandle);
  
  glGenTextures(1, &tex);

  NATIVEwindow *window;
#ifndef LUMIN
  window = nullptr;
#else
  window = windowsystem::CreateNativeWindow(width, height, true, gl->windowHandle);
#endif
  
  QueueOnBrowserThreadFront([&]() -> void {
    this->loadImmediate(url, window, width, height);

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
  Nan::SetMethod(proto, "load", Load);
  // Nan::SetMethod(proto, "resize", Resize);
  Nan::SetAccessor(proto, JS_STR("width"), WidthGetter, WidthSetter);
  Nan::SetAccessor(proto, JS_STR("height"), HeightGetter, HeightSetter);
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
  Nan::SetMethod(proto, "destroy", Destroy);

  Local<Function> ctorFn = ctor->GetFunction();
  Nan::SetMethod(ctorFn, "updateAll", UpdateAll);

  return scope.Escape(ctorFn);
}

NAN_METHOD(Browser::New) {
  if (
    info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("WebGLRenderingContext")) &&
    info[1]->IsNumber() &&
    info[2]->IsNumber() &&
    info[3]->IsString() &&
    info[4]->IsString()
  ) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
    int width = info[1]->Int32Value();
    int height = info[2]->Int32Value();
    String::Utf8Value urlUtf8Value(Local<String>::Cast(info[3]));
    std::string url(*urlUtf8Value, urlUtf8Value.length());
    String::Utf8Value dataPathValue(Local<String>::Cast(info[4]));
    std::string dataPath(*dataPathValue, dataPathValue.length());

    if (!embeddedInitialized) {
      browserThread = std::thread([dataPath{std::move(dataPath)}]() -> void {
        // std::cout << "initialize web core manager 1" << std::endl;
        const bool success = initializeEmbedded(dataPath);
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
          std::cerr << "Browser::Browser: failed to initialize embedded browser" << std::endl;
        }
      });
      embeddedInitialized = true;
    }

    Browser *browser = new Browser(gl, width, height, url);
    Local<Object> browserObj = info.This();
    browser->Wrap(browserObj);
    
    Nan::SetAccessor(browserObj, JS_STR("texture"), TextureGetter);

    return info.GetReturnValue().Set(browserObj);
  } else {
    return Nan::ThrowError("Browser::New: invalid arguments");
  }
}

void Browser::load(const std::string &url) {
  QueueOnBrowserThreadFront([&]() -> void {
    this->loadImmediate(url);
  
    uv_sem_post(&constructSem);
  });
  
  uv_sem_wait(&constructSem);
}

void Browser::loadImmediate(const std::string &url, NATIVEwindow *window, int width, int height) {
  browser_ = createEmbedded(
    url,
    gl,
    window,
    tex,
    width,
    height,
    &textureWidth,
    &textureHeight,
    [this]() -> EmbeddedBrowser {
      return this->browser_;
    },
    [this](EmbeddedBrowser browser_) -> void {
      this->browser_ = browser_;
    },
    [this]() -> void {
      QueueOnMainThread([this]() -> void {
        Nan::HandleScope scope;
        
        if (!this->onloadstart.IsEmpty()) {
          Local<Function> onloadstart = Nan::New(this->onloadstart);
          onloadstart->Call(Nan::Null(), 0, nullptr);
        }
      });
    },
    [this](const std::string &url) -> void {
      QueueOnMainThread([this, url]() -> void {
        Nan::HandleScope scope;
        
        if (!this->onloadend.IsEmpty()) {
          Local<Value> argv[] = {
            JS_STR(url),
          };
          Local<Function> onloadend = Nan::New(this->onloadend);
          onloadend->Call(Nan::Null(), sizeof(argv)/sizeof(argv[0]), argv);
        }
      });
    },
    [this](int errorCode, const std::string &errorString, const std::string &failedUrl) -> void {
      QueueOnMainThread([this, errorCode, errorString, failedUrl]() -> void {
        Nan::HandleScope scope;
        
        if (!this->onloaderror.IsEmpty()) {
          Local<Function> onloaderror = Nan::New(this->onloaderror);
          Local<Value> argv[] = {
            JS_INT(errorCode),
            JS_STR(errorString),
            JS_STR(failedUrl),
          };
          onloaderror->Call(Nan::Null(), sizeof(argv)/sizeof(argv[0]), argv);
        }
      });
    },
    [this](const std::string &jsString, const std::string &scriptUrl, int startLine) -> void {
      QueueOnMainThread([this, jsString, scriptUrl, startLine]() -> void {
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
      QueueOnMainThread([this, m]() -> void {
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
  );
}

/* void Browser::resize(int w, int h) {
  ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->resize(w, h);
	browser_->GetHost()->WasResized();
} */

NAN_METHOD(Browser::UpdateAll) {
  if (embeddedInitialized) {
    QueueOnBrowserThread([]() -> void {
      // std::cout << "browser update 1" << std::endl;
      embeddedDoMessageLoopWork();
      // std::cout << "browser update 2" << std::endl;
    });
  }
}

NAN_METHOD(Browser::Load) {
  if (info[0]->IsString()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    String::Utf8Value urlUtf8Value(Local<String>::Cast(info[0]));
    std::string url(*urlUtf8Value, urlUtf8Value.length());
    
    browser->load(url);
  } else {
    return Nan::ThrowError("Browser::Load: invalid arguments");
  }
}

NAN_GETTER(Browser::WidthGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int width = getEmbeddedWidth(browser->browser_);
  Local<Integer> widthValue = Nan::New<Integer>(width);
  info.GetReturnValue().Set(widthValue);
}
NAN_SETTER(Browser::WidthSetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  
  int width = value->Int32Value();
  
  QueueOnBrowserThread([browser, width]() -> void {
    setEmbeddedHeight(browser->browser_, width);
  });
}
NAN_GETTER(Browser::HeightGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int height = getEmbeddedHeight(browser->browser_);
  Local<Integer> heightValue = Nan::New<Integer>(height);
  info.GetReturnValue().Set(heightValue);
}
NAN_SETTER(Browser::HeightSetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  
  int height = value->Int32Value();
  
  QueueOnBrowserThread([browser, height]() -> void {
    setEmbeddedHeight(browser->browser_, height);
  });
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
  
  QueueOnBrowserThread([browser]() -> void {
    embeddedGoBack(browser->browser_);
  });
}

NAN_METHOD(Browser::Forward) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  
  QueueOnBrowserThread([browser]() -> void {
    embeddedGoForward(browser->browser_);
  });
}

NAN_METHOD(Browser::Reload) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  
  QueueOnBrowserThread([browser]() -> void {
    embeddedReload(browser->browser_);
  });
}

NAN_METHOD(Browser::SendMouseMove) {
  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int x = info[0]->Int32Value();
    int y = info[1]->Int32Value();
    
    QueueOnBrowserThread([browser, x, y]() -> void {
      embeddedMouseMove(browser->browser_, x, y);
    });
  } else {
    return Nan::ThrowError("Browser::SendMouseMove: invalid arguments");
  }
}

NAN_METHOD(Browser::SendMouseDown) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int x = info[0]->Int32Value();
    int y = info[1]->Int32Value();
    int button = info[2]->Int32Value();
    
    QueueOnBrowserThread([browser, x, y, button]() -> void {
      embeddedMouseDown(browser->browser_, x, y, button);
    });
  } else {
    return Nan::ThrowError("Browser::SendMouseDown: invalid arguments");
  }
}

NAN_METHOD(Browser::SendMouseUp) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int x = info[0]->Int32Value();
    int y = info[1]->Int32Value();
    int button = info[2]->Int32Value();
    
    QueueOnBrowserThread([browser, x, y, button]() -> void {
      embeddedMouseUp(browser->browser_, x, y, button);
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
    
    QueueOnBrowserThread([browser, x, y, deltaX, deltaY]() -> void {
      embeddedMouseWheel(browser->browser_, x, y, deltaX, deltaY);
    });
  } else {
    return Nan::ThrowError("Browser::SendMouseUp: invalid arguments");
  }
}

int GetKeyModifiers(Local<Object> modifiersObj) {
  int modifiers = 0;
  if (modifiersObj->Get(JS_STR("shiftKey"))->BooleanValue()) {
    modifiers |= (int)EmbeddedKeyModifiers::SHIFT;
  }
  if (modifiersObj->Get(JS_STR("ctrlKey"))->BooleanValue()) {
    modifiers |= (int)EmbeddedKeyModifiers::CTRL;
  }
  if (modifiersObj->Get(JS_STR("altKey"))->BooleanValue()) {
    modifiers |= (int)EmbeddedKeyModifiers::ALT;
  }
  return modifiers;
}

map<int, int> keyCodesMap{ 
	{ 57, 40 }, // ( -
	{ 48, 41 }, // )
	{ 56, 42 }, // *
	{ 61, 43 }, // +
	{ 53, 37 }, // % - 
	{ 55, 38 }, // & -
	{ 49, 33 }, // ! -
	{ 52, 36 }, // $ -
	{ 39, 34 }, // " -
	{ 51, 35 }, // # -
	{ 44, 60 }, // < 
	{ 46, 62 }, // > 
	{ 59, 58 }, // : 
	{ 47, 63 }, // ?
	{ 50, 64 }, // @
	{ 54, 94 }, // ^
	{ 91, 123 }, // {
	{ 93, 125 }, // }
	{ 45, 95 }, // _
	{ 92, 124 }, // |
	{ 96, 126 } // ~
};

int MutateKey(int key, Local<Object> modifiersObj){
  if (modifiersObj->Get(JS_STR("shiftKey"))->BooleanValue()){
    if (
      key >= 97 && // a
      key <= 122 // z
    ) {
      key -= 32;
    } else if(keyCodesMap.count(key)){
      key = keyCodesMap[key];
    }
  }
  return key;
}


NAN_METHOD(Browser::SendKeyDown) {
  // Nan::HandleScope scope;
  if (info[0]->IsNumber() && info[1]->IsObject()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int key = info[0]->Int32Value();
	
    Local<Object> modifiersObj = Local<Object>::Cast(info[1]);
    int modifiers = GetKeyModifiers(modifiersObj);
    int wkey = MutateKey(key, modifiersObj);
    
    QueueOnBrowserThread([browser, key, wkey, modifiers]() -> void {
      embeddedKeyDown(browser->browser_, key, wkey, modifiers);
    });
  } else {
    return Nan::ThrowError("Browser::SendKeyDown: invalid arguments");
  }
}

NAN_METHOD(Browser::SendKeyUp) {
  // Nan::HandleScope scope;
  if (info[0]->IsNumber() && info[1]->IsObject()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int key = info[0]->Int32Value();
	
    Local<Object> modifiersObj = Local<Object>::Cast(info[1]);
    int modifiers = GetKeyModifiers(modifiersObj);
    int wkey = MutateKey(key, modifiersObj);
    
    QueueOnBrowserThread([browser, key, wkey, modifiers]() -> void {
      embeddedKeyUp(browser->browser_, key, wkey, modifiers);
    });

  } else {
    return Nan::ThrowError("Browser::SendKeyUp: invalid arguments");
  }
}

NAN_METHOD(Browser::SendKeyPress) {
  // Nan::HandleScope scope;fbrowser_
  if (info[0]->IsNumber() && info[1]->IsObject()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int key = info[0]->Uint32Value();
	
    Local<Object> modifiersObj = Local<Object>::Cast(info[1]);
    int modifiers = GetKeyModifiers(modifiersObj);
    int wkey = MutateKey(key, modifiersObj);

    QueueOnBrowserThread([browser, key, wkey, modifiers]() -> void {
      embeddedKeyPress(browser->browser_, key, wkey, modifiers);
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
    
    QueueOnBrowserThread([browser, jsString, scriptUrl, startLine]() -> void {
      embeddedRunJs(browser->browser_, jsString, scriptUrl, startLine);

      // uv_sem_post(&constructSem);
    });
    
    // uv_sem_wait(&constructSem);
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

    QueueOnBrowserThread([browser, messageJson]() -> void {
      std::string jsString(postMessagePrefix);
      jsString += messageJson;
      jsString += postMessageSuffix;
      
      embeddedRunJs(browser->browser_, jsString, "<postMessage>", 1);
    });
  } else {
    return Nan::ThrowError("Browser::RunJs: invalid arguments");
  }
}

NAN_METHOD(Browser::Destroy) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  destroyEmbedded(browser->browser_);
  browser->browser_ = nullptr;
}

NAN_GETTER(Browser::TextureGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  Local<Object> textureObj = Nan::New<Object>();
  textureObj->Set(JS_STR("id"), JS_INT(browser->tex));
  info.GetReturnValue().Set(textureObj);
}

}

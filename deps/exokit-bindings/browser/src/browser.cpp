#include <browser.h>

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <exout>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

// Browser

Browser::Browser(WebGLRenderingContext *gl, int width, int height) : gl(gl), window(nullptr), width(width), height(height), tex(0), textureWidth(0), textureHeight(0) {
  windowsystem::SetCurrentWindowContext(gl->windowHandle);

  glGenTextures(1, &tex);

#ifdef LUMIN
  window = windowsystem::CreateNativeWindow(width, height, true, gl->windowHandle);
#endif
}

Browser::~Browser() {}

Local<Object> Browser::Initialize(Isolate *isolate) {
  uv_async_init(uv_default_loop(), &mainThreadAsync, MainThreadAsync);
  // uv_sem_init(&constructSem, 0);
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
  Nan::SetMethod(proto, "resize", Resize);
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

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();
  Nan::SetMethod(ctorFn, "updateAll", UpdateAll);

  return scope.Escape(ctorFn);
}

NAN_METHOD(Browser::New) {
  if (
    info[0]->IsObject() && JS_OBJ(JS_OBJ(info[0])->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("WebGLRenderingContext")) &&
    info[1]->IsNumber() &&
    info[2]->IsNumber() &&
    info[3]->IsString()
  ) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
    int width = TO_INT32(info[1]);
    int height = TO_INT32(info[2]);
    Nan::Utf8String dataPathValue(Local<String>::Cast(info[3]));
    std::string dataPath(*dataPathValue, dataPathValue.length());
    Nan::Utf8String frameworkPathValue(Local<String>::Cast(info[4]));
    std::string frameworkPath(*frameworkPathValue, frameworkPathValue.length());

    if (!embeddedInitialized) {
      browserThread = std::thread([dataPath{std::move(dataPath)}, frameworkPath{std::move(frameworkPath)}]() -> void {
        // exout << "initialize web core manager 1" << std::endl;
        const bool success = initializeEmbedded(dataPath, frameworkPath);
        // exout << "initialize web core manager 2 " << success << std::endl;
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
          exerr << "Browser::Browser: failed to initialize embedded browser" << std::endl;
        }
      });
      embeddedInitialized = true;
    }

    Browser *browser = new Browser(gl, width, height);
    Local<Object> browserObj = info.This();
    browser->Wrap(browserObj);
    
    Nan::SetAccessor(browserObj, JS_STR("texture"), TextureGetter);

    return info.GetReturnValue().Set(browserObj);
  } else {
    return Nan::ThrowError("Browser::New: invalid arguments");
  }
}

void Browser::load(const std::string &url) {
  QueueOnBrowserThread([this, url]() -> void {
    this->loadImmediate(url);
  
    // uv_sem_post(&constructSem);
  });
  
  // uv_sem_wait(&constructSem);
}

void Browser::loadImmediate(const std::string &url) {
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
          onloadstart->Call(Isolate::GetCurrent()->GetCurrentContext(), Nan::Null(), 0, nullptr);
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
          onloadend->Call(Isolate::GetCurrent()->GetCurrentContext(), Nan::Null(), sizeof(argv)/sizeof(argv[0]), argv);
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
          onloaderror->Call(Isolate::GetCurrent()->GetCurrentContext(), Nan::Null(), sizeof(argv)/sizeof(argv[0]), argv);
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
          onconsole->Call(Isolate::GetCurrent()->GetCurrentContext(), Nan::Null(), sizeof(argv)/sizeof(argv[0]), argv);
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
          onmessage->Call(Isolate::GetCurrent()->GetCurrentContext(), Nan::Null(), sizeof(argv)/sizeof(argv[0]), argv);
        }
      });
    }
  );
}

NAN_METHOD(Browser::UpdateAll) {
  if (embeddedInitialized) {
    embeddedUpdate();
    
    QueueOnBrowserThread([]() -> void {
      embeddedDoMessageLoopWork();
    });
  }
}

NAN_METHOD(Browser::Load) {
  if (info[0]->IsString()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    Nan::Utf8String urlUtf8(Local<String>::Cast(info[0]));
    std::string url(*urlUtf8, urlUtf8.length());
    
    browser->load(url);
  } else {
    return Nan::ThrowError("Browser::Load: invalid arguments");
  }
}

NAN_METHOD(Browser::Resize) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int width = TO_INT32(info[0]);
  int height = TO_INT32(info[1]);

  setEmbeddedSize(browser->browser_, width, height);
}

NAN_GETTER(Browser::WidthGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  if (browser->browser_) {
    int width = getEmbeddedWidth(browser->browser_);
    Local<Integer> widthValue = Nan::New<Integer>(width);
    info.GetReturnValue().Set(widthValue);
  } else {
    info.GetReturnValue().Set(Nan::New<Integer>(0));
  }
}
NAN_SETTER(Browser::WidthSetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  if (browser->browser_) {
    int width = TO_INT32(value);
    
    QueueOnBrowserThread([browser, width]() -> void {
      setEmbeddedWidth(browser->browser_, width);
    });
  }
}
NAN_GETTER(Browser::HeightGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  if (browser->browser_) {
    int height = getEmbeddedHeight(browser->browser_);
    Local<Integer> heightValue = Nan::New<Integer>(height);
    info.GetReturnValue().Set(heightValue);
  } else {
    info.GetReturnValue().Set(Nan::New<Integer>(0));
  }
}
NAN_SETTER(Browser::HeightSetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  if (browser->browser_) {
    int height = TO_INT32(value);

    QueueOnBrowserThread([browser, height]() -> void {
      setEmbeddedHeight(browser->browser_, height);
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
    if (browser->browser_) {
      int x = TO_INT32(info[0]);
      int y = TO_INT32(info[1]);
      
      QueueOnBrowserThread([browser, x, y]() -> void {
        embeddedMouseMove(browser->browser_, x, y);
      });
    }
  } else {
    return Nan::ThrowError("Browser::SendMouseMove: invalid arguments");
  }
}

NAN_METHOD(Browser::SendMouseDown) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    if (browser->browser_) {
      int x = TO_INT32(info[0]);
      int y = TO_INT32(info[1]);
      int button = TO_INT32(info[2]);
      
      QueueOnBrowserThread([browser, x, y, button]() -> void {
        embeddedMouseDown(browser->browser_, x, y, button);
      });
    }
  } else {
    return Nan::ThrowError("Browser::SendMouseDown: invalid arguments");
  }
}

NAN_METHOD(Browser::SendMouseUp) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    int x = TO_INT32(info[0]);
    int y = TO_INT32(info[1]);
    int button = TO_INT32(info[2]);
    
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
    if (browser->browser_) {
      int x = TO_INT32(info[0]);
      int y = TO_INT32(info[1]);
      int deltaX = TO_INT32(info[2]);
      int deltaY = TO_INT32(info[3]);
      
      QueueOnBrowserThread([browser, x, y, deltaX, deltaY]() -> void {
        embeddedMouseWheel(browser->browser_, x, y, deltaX, deltaY);
      });
    }
  } else {
    return Nan::ThrowError("Browser::SendMouseUp: invalid arguments");
  }
}

int GetKeyModifiers(Local<Object> modifiersObj) {
  int modifiers = 0;
  if (TO_BOOL(modifiersObj->Get(JS_STR("shiftKey")))) {
    modifiers |= (int)EmbeddedKeyModifiers::SHIFT;
  }
  if (TO_BOOL(modifiersObj->Get(JS_STR("ctrlKey")))) {
    modifiers |= (int)EmbeddedKeyModifiers::CTRL;
  }
  if (TO_BOOL(modifiersObj->Get(JS_STR("altKey")))) {
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
  if (TO_BOOL(modifiersObj->Get(JS_STR("shiftKey")))){
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
    if (browser->browser_) {
      int key = TO_INT32(info[0]);
    
      Local<Object> modifiersObj = Local<Object>::Cast(info[1]);
      int modifiers = GetKeyModifiers(modifiersObj);
      int wkey = MutateKey(key, modifiersObj);
      
      QueueOnBrowserThread([browser, key, wkey, modifiers]() -> void {
        embeddedKeyDown(browser->browser_, key, wkey, modifiers);
      });
    }
  } else {
    return Nan::ThrowError("Browser::SendKeyDown: invalid arguments");
  }
}

NAN_METHOD(Browser::SendKeyUp) {
  // Nan::HandleScope scope;
  if (info[0]->IsNumber() && info[1]->IsObject()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    if (browser->browser_) {
      int key = TO_INT32(info[0]);
    
      Local<Object> modifiersObj = Local<Object>::Cast(info[1]);
      int modifiers = GetKeyModifiers(modifiersObj);
      int wkey = MutateKey(key, modifiersObj);
      
      QueueOnBrowserThread([browser, key, wkey, modifiers]() -> void {
        embeddedKeyUp(browser->browser_, key, wkey, modifiers);
      });
    }
  } else {
    return Nan::ThrowError("Browser::SendKeyUp: invalid arguments");
  }
}

NAN_METHOD(Browser::SendKeyPress) {
  // Nan::HandleScope scope;fbrowser_
  if (info[0]->IsNumber() && info[1]->IsObject()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    if (browser->browser_) {
      int key = TO_UINT32(info[0]);
    
      Local<Object> modifiersObj = Local<Object>::Cast(info[1]);
      int modifiers = GetKeyModifiers(modifiersObj);
      int wkey = MutateKey(key, modifiersObj);

      QueueOnBrowserThread([browser, key, wkey, modifiers]() -> void {
        embeddedKeyPress(browser->browser_, key, wkey, modifiers);
      });
    }
  } else {
    return Nan::ThrowError("Browser::SendKeyPress: invalid arguments");
  }
}

NAN_METHOD(Browser::RunJs) {
  if (info[0]->IsString() && info[1]->IsString() && info[2]->IsNumber()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
    
    if (browser->browser_) {
      Nan::Utf8String jsStringValue(Local<String>::Cast(info[0]));
      string jsString(*jsStringValue, jsStringValue.length());
      
      Nan::Utf8String scriptUrlValue(Local<String>::Cast(info[1]));
      string scriptUrl(*scriptUrlValue, scriptUrlValue.length());

      int startLine = TO_INT32(info[2]);
      
      QueueOnBrowserThread([browser, jsString, scriptUrl, startLine]() -> void {
        embeddedRunJs(browser->browser_, jsString, scriptUrl, startLine);

        // uv_sem_post(&constructSem);
      });
      
      // uv_sem_wait(&constructSem);
    }
  } else {
    return Nan::ThrowError("Browser::RunJs: invalid arguments");
  }
}

const char *postMessagePrefix = "window.dispatchEvent(new MessageEvent('message', {data: ";
const char *postMessageSuffix = "}));";
NAN_METHOD(Browser::PostMessage) {
  if (info[0]->IsString()) {
    Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

    if (browser->browser_) {
      Nan::Utf8String messageJsonValue(Local<String>::Cast(info[0]));
      string messageJson(*messageJsonValue, messageJsonValue.length());

      QueueOnBrowserThread([browser, messageJson]() -> void {
        std::string jsString(postMessagePrefix);
        jsString += messageJson;
        jsString += postMessageSuffix;
        
        embeddedRunJs(browser->browser_, jsString, "<postMessage>", 1);
      });
    }
  } else {
    return Nan::ThrowError("Browser::RunJs: invalid arguments");
  }
}

NAN_METHOD(Browser::Destroy) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  if (browser->browser_) {
    destroyEmbedded(browser->browser_);
    browser->browser_ = nullptr;
  }
}

NAN_GETTER(Browser::TextureGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  Local<Object> textureObj = Nan::New<Object>();
  textureObj->Set(JS_STR("id"), JS_INT(browser->tex));
  info.GetReturnValue().Set(textureObj);
}

}

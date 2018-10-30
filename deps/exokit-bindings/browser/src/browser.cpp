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

// RenderHandler

RenderHandler::RenderHandler(OnPaintFn onPaint) : onPaint(onPaint), width(2), height(2) {}

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

BrowserClient::BrowserClient(RenderHandler *renderHandler) : m_renderHandler(renderHandler) {}

BrowserClient::~BrowserClient() {}

// Browser

Browser::Browser(WebGLRenderingContext *gl, int width, int height, const std::string &url) : initialized(false) {
  glGenTextures(1, &tex);
  
  render_handler_.reset(new RenderHandler([this, gl](const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) -> void {
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
	render_handler_->resize(128, 128);

	CefWindowInfo window_info;
  window_info.SetAsWindowless(nullptr);
	CefBrowserSettings browserSettings;
	// browserSettings.windowless_frame_rate = 60; // 30 is default
	client_.reset(new BrowserClient(render_handler_.get()));

	browser_ = CefBrowserHost::CreateBrowserSync(window_info, client_.get(), url, browserSettings, nullptr);
  
  this->reshape(width, height);
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
    info[1]->IsNumber() &&
    info[2]->IsNumber() &&
    info[3]->IsString()
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

void Browser::Update() {
  CefDoMessageLoopWork();
}

void Browser::reshape(int w, int h) {
	render_handler_->resize(w, h);
	browser_->GetHost()->WasResized();
}

NAN_METHOD(Browser::Update) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  // std::cout << "browser update 1" << std::endl;
  browser->Update();
  // std::cout << "browser update 2" << std::endl;
}

NAN_GETTER(Browser::TextureGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  Local<Object> textureObj = Nan::New<Object>();
  textureObj->Set(JS_STR("id"), JS_INT(browser->tex));
  info.GetReturnValue().Set(textureObj);
}

/* WebCore::WebCore(const std::string &url, RenderHandler::OnPaintFn onPaint)
	: mouse_x_(0), mouse_y_(0)
{
	render_handler_ = new RenderHandler(onPaint);
	render_handler_->init();
	// initial size
	render_handler_->resize(128, 128);

	CefWindowInfo window_info;
	// HWND hwnd = GetConsoleWindow();
	// window_info.SetAsWindowless(hwnd, true);
  window_info.SetAsWindowless(nullptr);

	CefBrowserSettings browserSettings;
	// browserSettings.windowless_frame_rate = 60; // 30 is default
	client_ = new BrowserClient(render_handler_);

	browser_ = CefBrowserHost::CreateBrowserSync(window_info, client_.get(), url, browserSettings, nullptr);
}

WebCore::~WebCore()
{
	browser_->GetHost()->CloseBrowser(true);
	CefDoMessageLoopWork();

	browser_ = nullptr;
	client_ = nullptr;
}

void WebCore::reshape(int w, int h)
{
	render_handler_->resize(w, h);
	browser_->GetHost()->WasResized();
}


void WebCore::mouseMove(int x, int y)
{
	mouse_x_ = x;
	mouse_y_ = y;

	CefMouseEvent evt;
	evt.x = x;
	evt.y = y;

	//TODO
	bool mouse_leave = false;

	browser_->GetHost()->SendMouseMoveEvent(evt, mouse_leave);
}

void WebCore::mouseClick(CefBrowserHost::MouseButtonType btn, bool mouse_up)
{
	CefMouseEvent evt;
	evt.x = mouse_x_;
	evt.y = mouse_y_;

	//TODO
	int click_count = 1;

	browser_->GetHost()->SendMouseClickEvent(evt, btn, mouse_up, click_count);
}

void WebCore::keyPress(int key, bool pressed)
{
	//TODO ???
	// test page http://javascript.info/tutorial/keyboard-events
	CefKeyEvent evt;
	//event.native_key_code = key;
	//event.type = pressed ? KEYEVENT_KEYDOWN : KEYEVENT_KEYUP;
	evt.character = key;
	evt.native_key_code = key;
	evt.type = KEYEVENT_CHAR;

	browser_->GetHost()->SendKeyEvent(evt);
} */

bool cefInitialized = false;

}

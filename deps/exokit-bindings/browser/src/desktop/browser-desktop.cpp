#ifndef LUMIN

#include <desktop/browser-desktop.h>

using namespace std;
using namespace v8;
using namespace node;

// helpers

namespace browser {

void CefDoMessageLoopWork() {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_do_message_loop_work();
}

CefRefPtr<CefBrowser> CreateBrowserSync(
    const CefWindowInfo& windowInfo,
    CefRefPtr<CefClient> client,
    const CefString& url,
    const CefBrowserSettings& settings,
    CefRefPtr<CefRequestContext> request_context) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: client, url, request_context

  // Execute
  cef_browser_t* _retval = cef_browser_host_create_browser_sync(
      &windowInfo, CefClientCppToC::Wrap(client), url.GetStruct(), &settings,
      CefRequestContextCToCpp::Unwrap(request_context));

  // Return type: refptr_same
  return CefBrowserCToCpp::Wrap(_retval);
}

/* bool initializeEmbedded(const std::string &dataPath) {
  CefMainArgs args;
  
	CefSettings settings;
  // settings.log_severity = LOGSEVERITY_VERBOSE;
  // CefString(&settings.resources_dir_path) = resourcesPath;
  // CefString(&settings.locales_dir_path) = localesPath;
  CefString(&settings.cache_path).FromString(dataPath);
  CefString(&settings.log_file).FromString(dataPath + "/log.txt");
  settings.no_sandbox = true;
  
  SimpleApp *app = new SimpleApp(dataPath);
  
	return CefInitialize(args, settings, app, nullptr);
}

void embeddedDoMessageLoopWork() {
  CefDoMessageLoopWork();
} */

EmbeddedBrowser createEmbedded(
  EmbeddedBrowser browser_,
  const std::string &url,
  WebGLRenderingContext *gl,
  NATIVEwindow *window,
  GLuint tex,
  int *textureWidth,
  int *textureHeight,
  int width,
  int height,
  std::function<void()> onloadstart,
  std::function<void(const std::string &)> onloadend,
  std::function<void(int, const std::string &, const std::string &)> onloaderror,
  std::function<void(const std::string &, const std::string &, int)> onconsole,
  std::function<void(const std::string &)> onmessage
) {
  if (width == 0) {
    width = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->width;
  }
  if (height == 0) {
    height = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->height;
  }
  
  if (browser_) {
    browser_->GetHost()->CloseBrowser(true);
    browser_ = nullptr;
    
    *textureWidth = 0;
    *textureHeight = 0;
  }
  
  LoadHandler *load_handler_ = new LoadHandler(
    [browser_, onloadstart]() -> void {
      browser_->GetMainFrame()->ExecuteJavaScript(CefString("window.postMessage = m => {console.log('<postMessage>' + JSON.stringify(m));};"), CefString("<bootstrap>"), 1);
      
      onloadstart();
    },
    [browser_, onloadend]() -> void {
      CefString loadUrl = browser_->GetMainFrame()->GetURL();
      onloadend(loadUrl.ToString());
    },
    [onloaderror](int errorCode, const std::string &errorString, const std::string &failedUrl) -> void {
      onloaderror(errorCode, errorString, failedUrl);
    }
  );
  
  DisplayHandler *display_handler_ = new DisplayHandler(
    [onconsole](const std::string &jsString, const std::string &scriptUrl, int startLine) -> void {
      onconsole(jsString, scriptUrl, startLine);
    },
    [onmessage](const std::string &m) -> void {
      onmessage(m);
    }
  );
  
  RenderHandler *render_handler_ = new RenderHandler(
    [gl, tex, textureWidth, textureHeight, width, height](const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) -> void {
      RunOnMainThread([&]() -> void {
        windowsystem::SetCurrentWindowContext(gl->windowHandle);
        
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, width); // XXX save/restore these

        if (*textureWidth != width || *textureHeight != height) {
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
          glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
// #ifndef LUMIN
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
// #else
          // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
// #endif
        
          *textureWidth = width;
          *textureHeight = height;
        }

        for (size_t i = 0; i < dirtyRects.size(); i++) {
          const CefRect &rect = dirtyRects[i];
          
          glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect.x);
          glPixelStorei(GL_UNPACK_SKIP_ROWS, rect.y);
// #ifndef LUMIN
          glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
// #else
          // glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width, rect.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, buffer);
// #endif
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
    },
    width,
    height
  );
  
  CefWindowInfo window_info;
  window_info.SetAsWindowless((CefWindowHandle)NULL);
  CefBrowserSettings browserSettings;
  // browserSettings.windowless_frame_rate = 60; // 30 is default
  BrowserClient *client = new BrowserClient(load_handler_, display_handler_, render_handler_);
  
  return CreateBrowserSync(window_info, client, url, browserSettings, nullptr);
}
void destroyEmbedded(EmbeddedBrowser browser_) {
  // nothing
}
int getEmbeddedWidth(EmbeddedBrowser browser_) {
  return ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->width;
}
void setEmbeddedWidth(EmbeddedBrowser browser_, int width) {
  ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->width = width;
  
  browser_->GetHost()->WasResized();
  // browser->browser_->GetHost()->Invalidate(PET_VIEW);
}
int getEmbeddedHeight(EmbeddedBrowser browser_) {
  return ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->height;
}
void setEmbeddedHeight(EmbeddedBrowser browser_, int height) {
  ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->height = height;
  
  browser_->GetHost()->WasResized();
  // browser->browser_->GetHost()->Invalidate(PET_VIEW);
}
void embeddedGoBack(EmbeddedBrowser browser_) {
  browser_->GoBack();
}
void embeddedGoForward(EmbeddedBrowser browser_) {
  browser_->GoForward();
}
void embeddedReload(EmbeddedBrowser browser_) {
  browser_->Reload();
}
void embeddedMouseMove(EmbeddedBrowser browser_, int x, int y) {
  CefMouseEvent evt;
  evt.x = x;
  evt.y = y;

  browser_->GetHost()->SendMouseMoveEvent(evt, false);
}
void embeddedMouseDown(EmbeddedBrowser browser_, int x, int y, int button) {
  CefMouseEvent evt;
  evt.x = x;
  evt.y = y;

  browser_->GetHost()->SendMouseClickEvent(evt, button, false, 1);
}
void embeddedMouseUp(EmbeddedBrowser browser_, int x, int y, int button) {
  CefMouseEvent evt;
  evt.x = x;
  evt.y = y;

  browser_->GetHost()->SendMouseClickEvent(evt, button, true, 1);
}
void embeddedMouseWheel(EmbeddedBrowser browser_, int x, int y, int deltaX, int deltaY) {
  CefMouseEvent evt;
  evt.x = x;
  evt.y = y;

  browser_->GetHost()->SendMouseWheelEvent(evt, deltaX, deltaY);
}
int modifiers2int(int modifiers) {
  int result;
  if (modifiers & (int)EmbeddedKeyModifiers::SHIFT) {
    result |= EVENTFLAG_SHIFT_DOWN;
  }
  if (modifiers & (int)EmbeddedKeyModifiers::CTRL) {
    result |= EVENTFLAG_CONTROL_DOWN; // EVENTFLAG_COMMAND_DOWN  for mac?
  }
  if (modifiers & (int)EmbeddedKeyModifiers::ALT) {
    result |= EVENTFLAG_ALT_DOWN;
  }
  return result;
}
void embeddedKeyDown(EmbeddedBrowser browser_, int key, int modifiers) {
  CefKeyEvent evt;
  evt.type = KEYEVENT_RAWKEYDOWN;
  evt.character = key;
  evt.native_key_code = key;
  evt.windows_key_code = key;
  evt.modifiers = modifiers2int(modifiers);
  
  browser_->GetHost()->SendKeyEvent(evt);
}
void embeddedKeyUp(EmbeddedBrowser browser_, int key, int modifiers) {
  CefKeyEvent evt;
  evt.type = KEYEVENT_KEYUP;
  evt.character = key;
  evt.native_key_code = key;
  evt.windows_key_code = key;
  evt.modifiers = modifiers2int(modifiers);

  browser_->GetHost()->SendKeyEvent(evt);
}
void embeddedKeyPress(EmbeddedBrowser browser_, int key, int wkey, int modifiers) {
  CefKeyEvent evt;
  evt.type = KEYEVENT_CHAR;
  evt.character = key;
  evt.native_key_code = key;
  evt.windows_key_code = wkey;
  evt.modifiers = modifiers2int(modifiers);

  browser_->GetHost()->SendKeyEvent(evt);
}
void embeddedRunJs(EmbeddedBrowser browser_, const std::string &jsString, const std::string &scriptUrl, int startLine) {
  browser_->GetMainFrame()->ExecuteJavaScript(CefString(jsString), CefString(scriptUrl), startLine);
}

CefBrowserHost::MouseButtonType GetMouseButton(int button){
	CefBrowserHost::MouseButtonType mouseButton;
	switch (button){
		case 0:
		 mouseButton = CefBrowserHost::MouseButtonType::MBT_LEFT;
		 break;
		case 1:
		 mouseButton = CefBrowserHost::MouseButtonType::MBT_MIDDLE;
		 break;
		case 2:
		 mouseButton = CefBrowserHost::MouseButtonType::MBT_RIGHT;
		 break;
	}
	return mouseButton;
}

bool CefInitialize(const CefMainArgs& args,
                              const CefSettings& settings,
                              CefRefPtr<CefApp> application,
                              void* windows_sandbox_info) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: application, windows_sandbox_info

  // Execute
  int _retval = cef_initialize(
      &args, &settings, CefAppCppToC::Wrap(application), windows_sandbox_info);

  // Return type: bool
  return _retval ? true : false;
}

// SimpleApp

SimpleApp::SimpleApp(const std::string &dataPath) : dataPath(dataPath) {}

void SimpleApp::OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line) {
  command_line->AppendSwitch(CefString("single-process"));
  // command_line->AppendSwitch(CefString("no-proxy-server"));
  command_line->AppendSwitch(CefString("winhttp-proxy-resolver"));
  command_line->AppendSwitch(CefString("no-sandbox"));
  CefString dataPathString(dataPath);
  command_line->AppendSwitchWithValue(CefString("user-data-dir"), dataPathString);
  command_line->AppendSwitchWithValue(CefString("disk-cache-dir"), dataPathString);
}

void SimpleApp::OnContextInitialized() {
  // CEF_REQUIRE_UI_THREAD();
  
  // std::cout << "SimpleApp::OnContextInitialized" << std::endl;
}

// LoadHandler

LoadHandler::LoadHandler(std::function<void()> onLoadStart, std::function<void()> onLoadEnd, std::function<void(int, const std::string &, const std::string &)> onLoadError) : onLoadStart(onLoadStart), onLoadEnd(onLoadEnd), onLoadError(onLoadError) {}

LoadHandler::~LoadHandler() {}

void LoadHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) {
  onLoadStart();
}

void LoadHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) {
  onLoadEnd();
}

void LoadHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString &errorText, const CefString &failedUrl) {
  onLoadError((int)errorCode, errorText.ToString(), failedUrl.ToString());
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

RenderHandler::RenderHandler(OnPaintFn onPaint, int width, int height) : onPaint(onPaint), width(width), height(height) {}

RenderHandler::~RenderHandler() {}

/* void RenderHandler::resize(int w, int h) {
	width = w;
	height = h;
} */

bool RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) {
	rect = CefRect(0, 0, width, height);
	return true;
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) {
  onPaint(dirtyRects, buffer, width, height);
}

// BrowserClient

BrowserClient::BrowserClient(LoadHandler *loadHandler, DisplayHandler *displayHandler, RenderHandler *renderHandler/*, LifeSpanHandler *lifespanHandler*/) :
  m_loadHandler(loadHandler), m_displayHandler(displayHandler), m_renderHandler(renderHandler)/*, m_lifespanHandler(lifespanHandler)*/ {}

BrowserClient::~BrowserClient() {}

};

#endif

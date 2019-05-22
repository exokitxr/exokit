#if defined(ANDROID) && !defined(LUMIN) 

/* #include <browser-android.h>

using namespace std;
using namespace v8;
using namespace node; */

// XXX finish this

/* auto textureName = 1; // Should be retrieved using glGenTextures() function
auto textureWidth = 512;
auto textureHeight = 512;

// Retrieve the JNI environment, using SDL, it looks like that
auto env = (JNIEnv*)SDL_AndroidGetJNIEnv();

// Create a SurfaceTexture using JNI
const jclass surfaceTextureClass = env->FindClass("android/graphics/SurfaceTexture");
// Find the constructor that takes an int (texture name)
const jmethodID surfaceTextureConstructor = env->GetMethodID(surfaceTextureClass, "<init>", "(I)V" );
jobject surfaceTextureObject = env->NewObject(surfaceTextureClass, surfaceTextureConstructor, textureName);
jobject jniSurfaceTexture = env->NewGlobalRef(surfaceTextureObject);

// To update the SurfaceTexture content
jmethodId updateTexImageMethodId = env->GetMethodID(surfaceTextureClass, "updateTexImage", "()V");
// To update the SurfaceTexture size
jmethodId setDefaultBufferSizeMethodId = env->GetMethodID(surfaceTextureClass, "setDefaultBufferSize", "(II)V" );

// Create a Surface from the SurfaceTexture using JNI
const jclass surfaceClass = env->FindClass("android/view/Surface");
const jmethodID surfaceConstructor = env->GetMethodID(surfaceClass, "<init>", "(Landroid/graphics/SurfaceTexture;)V");
jobject surfaceObject = env->NewObject(surfaceClass, surfaceConstructor, jniSurfaceTexture);
jobject jniSurface = env->NewGlobalRef(surfaceObject);

// Now that we have a globalRef, we can free the localRef
env->DeleteLocalRef(surfaceTextureObject);
env->DeleteLocalRef(surfaceTextureClass);
env->DeleteLocalRef(surfaceObject);
env->DeleteLocalRef(surfaceClass);

// Don't forget to update the size of the SurfaceTexture
env->CallVoidMethod(jniSurfaceTexture, setDefaultBufferSizeMethodId, textureWidth, textureHeight);

// Get the method to pass the Surface object to the WebView
jmethodId setWebViewRendererSurfaceMethod = env->GetMethodID(webViewClass, "setWebViewRendererSurface", "(Landroid/view/Surface;)V");
// Pass the JNI Surface object to the Webview
env->CallVoidMethod(webView, setWebViewRendererSurfaceMethod, jniSurface); */

// helpers

namespace browser {

/* constexpr double ZOOM_LOG = 1.15;

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

bool CefInitialize2(const CefMainArgs& args,
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
bool initializeEmbedded(const std::string &dataPath, const std::string &frameworkPath) {
#ifdef __APPLE__
  std::string libraryPath = frameworkPath + "/Chromium Embedded Framework";
  cef_load_library(libraryPath.c_str());
#endif

  CefMainArgs args;

	CefSettings settings;
  // settings.log_severity = LOGSEVERITY_VERBOSE;
  // CefString(&settings.resources_dir_path) = resourcesPath;
  // CefString(&settings.locales_dir_path) = localesPath;
  CefString(&settings.cache_path).FromString(dataPath);
  CefString(&settings.log_file).FromString(dataPath + "/log.txt");
#ifdef __APPLE__
  CefString(&settings.framework_dir_path).FromString(frameworkPath);
#endif
  settings.no_sandbox = true;
  // settings.multi_threaded_message_loop = false;
  settings.external_message_pump = true;
  
  SimpleApp *app = new SimpleApp(dataPath);
  
	return CefInitialize2(args, settings, app, nullptr);
}

void embeddedDoMessageLoopWork() {
  cef_do_message_loop_work();
}

EmbeddedBrowser createEmbedded(
  const std::string &url,
  WebGLRenderingContext *gl,
  NATIVEwindow *window,
  GLuint tex,
  int width,
  int height,
  float scale,
  int *textureWidth,
  int *textureHeight,
  std::function<EmbeddedBrowser()> getBrowser,
  std::function<void(EmbeddedBrowser)> setBrowser,
  std::function<void()> onloadstart,
  std::function<void(const std::string &)> onloadend,
  std::function<void(int, const std::string &, const std::string &)> onloaderror,
  std::function<void(const std::string &, const std::string &, int)> onconsole,
  std::function<void(const std::string &)> onmessage
) {
  EmbeddedBrowser browser_ = getBrowser();

  if (width == 0) {
    width = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->width;
  }
  if (height == 0) {
    height = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->height;
  }

  if (browser_) {
    browser_->GetHost()->CloseBrowser(true);
    setBrowser(nullptr);
    
    *textureWidth = 0;
    *textureHeight = 0;
    
    {
      std::lock_guard<std::mutex> lock(browsersMutex);

      browsers.erase(std::find(browsers.begin(), browsers.end(), browser_));
    }
  }
  
  LoadHandler *load_handler_ = new LoadHandler(
    [getBrowser, onloadstart, scale]() -> void {
      getBrowser()->GetMainFrame()->ExecuteJavaScript(CefString("window.postMessage = m => {console.log('<postMessage>' + JSON.stringify(m));};"), CefString("<bootstrap>"), 1);
      setEmbeddedScale(getBrowser(), scale);
      
      onloadstart();
    },
    [getBrowser, onloadend]() -> void {
      CefString loadUrl = getBrowser()->GetMainFrame()->GetURL();
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
    [window, tex, textureWidth, textureHeight](const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) -> void {
      // std::cout << "paint 1 " << tex << std::endl;
      
      windowsystem::SetCurrentWindowContext(window);

      glBindTexture(GL_TEXTURE_2D, tex);

      if (*textureWidth != width || *textureHeight != height) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // XXX save/restore these
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
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

      glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

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
      
      glFinish();
    },
    width,
    height,
    scale
  );
  
  CefWindowInfo window_info;
  window_info.SetAsWindowless((CefWindowHandle)NULL);
  CefBrowserSettings browserSettings;
  // browserSettings.windowless_frame_rate = 60; // 30 is default
  BrowserClient *client = new BrowserClient(load_handler_, display_handler_, render_handler_);
  
  EmbeddedBrowser result = CreateBrowserSync(window_info, client, url, browserSettings, nullptr);
  
  {
    std::lock_guard<std::mutex> lock(browsersMutex);

    browsers.push_back(result);
  }
  
  return result;
}
void destroyEmbedded(EmbeddedBrowser browser_) {
  {
    std::lock_guard<std::mutex> lock(browsersMutex);

    browsers.erase(std::find(browsers.begin(), browsers.end(), browser_));
  }
  
  browser_->GetHost()->CloseBrowser(false);
}
std::pair<int, int> getEmbeddedSize(EmbeddedBrowser browser_) {
  auto renderHandler = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler;
  return std::pair<int, int>(renderHandler->width, renderHandler->height);
}
void setEmbeddedSize(EmbeddedBrowser browser_, int width, int height) {
  auto renderHandler = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler;
  renderHandler->width = width;
  renderHandler->height = height;

  browser_->GetHost()->WasResized();
  setEmbeddedScale(browser_, getEmbeddedScale(browser_));
  // renderHandler->resized = true;
}
int getEmbeddedWidth(EmbeddedBrowser browser_) {
  return ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->width;
}
void setEmbeddedWidth(EmbeddedBrowser browser_, int width) {
  auto renderHandler = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler;
  renderHandler->width = width;
  browser_->GetHost()->WasResized();
  setEmbeddedScale(browser_, getEmbeddedScale(browser_));
  // renderHandler->resized = true;
  
  // browser_->GetHost()->WasResized();
  // browser->browser_->GetHost()->Invalidate(PET_VIEW);
}
int getEmbeddedHeight(EmbeddedBrowser browser_) {
  return ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler->height;
}
void setEmbeddedHeight(EmbeddedBrowser browser_, int height) {
  auto renderHandler = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler;
  renderHandler->height = height;
  browser_->GetHost()->WasResized();
  setEmbeddedScale(browser_, getEmbeddedScale(browser_));
  // renderHandler->resized = true;
  
  // browser_->GetHost()->WasResized();
  // browser->browser_->GetHost()->Invalidate(PET_VIEW);
}
float getEmbeddedScale(EmbeddedBrowser browser_) {
  auto renderHandler = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler;
  float scale = renderHandler->scale;
  return scale;
}
void setEmbeddedScale(EmbeddedBrowser browser_, float scale) {
  auto renderHandler = ((BrowserClient *)browser_->GetHost()->GetClient().get())->m_renderHandler;
  renderHandler->scale = scale;
  browser_->GetHost()->SetZoomLevel(log(scale)/log(ZOOM_LOG));
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

  browser_->GetHost()->SendMouseClickEvent(evt, GetMouseButton(button), false, 1);
}
void embeddedMouseUp(EmbeddedBrowser browser_, int x, int y, int button) {
  CefMouseEvent evt;
  evt.x = x;
  evt.y = y;

  browser_->GetHost()->SendMouseClickEvent(evt, GetMouseButton(button), true, 1);
}
void embeddedMouseWheel(EmbeddedBrowser browser_, int x, int y, int deltaX, int deltaY) {
  CefMouseEvent evt;
  evt.x = x;
  evt.y = y;

  browser_->GetHost()->SendMouseWheelEvent(evt, deltaX, deltaY);
}
int modifiers2int(int modifiers) {
  int result = 0;
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
void embeddedKeyDown(EmbeddedBrowser browser_, int key, int wkey, int modifiers) {
  CefKeyEvent evt = {};
  evt.type = KEYEVENT_RAWKEYDOWN;
  evt.character = key;
  evt.native_key_code = key;
  evt.windows_key_code = wkey;
  evt.unmodified_character = key;
  // evt.is_system_key = false;
  // evt.focus_on_editable_field = true;
  evt.modifiers = modifiers2int(modifiers);
  
  browser_->GetHost()->SendKeyEvent(evt);
}
void embeddedKeyUp(EmbeddedBrowser browser_, int key, int wkey, int modifiers) {
  CefKeyEvent evt = {};
  evt.type = KEYEVENT_KEYUP;
  evt.character = key;
  evt.native_key_code = key;
  evt.windows_key_code = wkey;
  evt.unmodified_character = key;
  // evt.is_system_key = false;
  // evt.focus_on_editable_field = true;
  evt.modifiers = modifiers2int(modifiers);

  browser_->GetHost()->SendKeyEvent(evt);
}
void embeddedKeyPress(EmbeddedBrowser browser_, int key, int wkey, int modifiers) {
  CefKeyEvent evt = {};
  evt.type = KEYEVENT_CHAR;
  evt.character = key;
  evt.native_key_code = key;
  evt.windows_key_code = wkey;
  evt.unmodified_character = key;
  // evt.is_system_key = false;
  // evt.focus_on_editable_field = true;
  evt.modifiers = modifiers2int(modifiers);

  browser_->GetHost()->SendKeyEvent(evt);
}
void embeddedRunJs(EmbeddedBrowser browser_, const std::string &jsString, const std::string &scriptUrl, int startLine) {
  browser_->GetMainFrame()->ExecuteJavaScript(CefString(jsString), CefString(scriptUrl), startLine);
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
}

void SimpleApp::OnScheduleMessagePumpWork(int64 delay_ms) {
  if (embeddedInitialized) {
    cef_do_message_loop_work();
  }
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

RenderHandler::RenderHandler(OnPaintFn onPaint, int width, int height, float scale) : onPaint(onPaint), width(width), height(height), scale(scale) {}

RenderHandler::~RenderHandler() {}

bool RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) {
	rect = CefRect(0, 0, width, height);
	return true;
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) {
  onPaint(dirtyRects, buffer, width, height);
}

// BrowserClient

BrowserClient::BrowserClient(LoadHandler *loadHandler, DisplayHandler *displayHandler, RenderHandler *renderHandler) :
  m_loadHandler(loadHandler), m_displayHandler(displayHandler), m_renderHandler(renderHandler) {}

BrowserClient::~BrowserClient() {}

std::mutex browsersMutex;
std::list<EmbeddedBrowser> browsers; */

}

#endif

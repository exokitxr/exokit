// Ŭnicode please
#include "web_core.h"
#include "browser_client.h"
#include "render_handler.h"

#include <iostream>

WebCoreManager::WebCoreManager()
{

}

WebCoreManager::~WebCoreManager()
{

}

bool WebCoreManager::setUp(int *exit_code)
{
	assert(exit_code != nullptr);

  /* const char *argv[] = {
    "node",
    "--single-process",
  };
  int argc = sizeof(argv)/sizeof(argv[0]);
	CefMainArgs args(argc, argc); */
  
	/* CefMainArgs args;
	*exit_code = CefExecuteProcess(args, nullptr, nullptr);
	if (*exit_code >= 0) { 
		return false;
	} */

  CefMainArgs args;
  
	CefSettings settings;
  // settings.log_severity = LOGSEVERITY_VERBOSE;
  // CefString(&settings.resources_dir_path) = resourcesPath;
  // CefString(&settings.locales_dir_path) = localesPath;
  settings.no_sandbox = true;
  
  // CefRefPtr<SimpleApp> app(new SimpleApp());
  SimpleApp *app = new SimpleApp();
  
	bool result = CefInitialize(args, settings, app, nullptr);
	if (!result) {
		*exit_code = -1;
		return false;
	}
	return true;
}

bool WebCoreManager::shutDown()
{
	browsers_.clear();
	CefShutdown();
	return true;
}

void WebCoreManager::update()
{
	CefDoMessageLoopWork();
}

std::weak_ptr<WebCore> WebCoreManager::createBrowser(const std::string &url, RenderHandler::OnPaintFn onPaint)
{
	auto web_core = std::make_shared<WebCore>(url, onPaint);
	browsers_.push_back(web_core);
	return web_core;
}

void WebCoreManager::removeBrowser(std::weak_ptr<WebCore> web_core)
{
	auto elem = web_core.lock();
	if (elem) {
		auto found = std::find(browsers_.begin(), browsers_.end(), elem);
		if (found != browsers_.end()) {
			browsers_.erase(found);
		}
	}
}

WebCore::WebCore(const std::string &url, RenderHandler::OnPaintFn onPaint)
	: mouse_x_(0), mouse_y_(0)
{
	render_handler_ = new RenderHandler(onPaint);
	render_handler_->init();
	// initial size
	render_handler_->resize(128, 128);

	CefWindowInfo window_info;
	/* HWND hwnd = GetConsoleWindow();
	window_info.SetAsWindowless(hwnd, true); */
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
}

SimpleApp::SimpleApp() {}

void SimpleApp::OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line) {
  command_line->AppendSwitch(CefString("single-process"));
  // command_line->AppendSwitch(CefString("no-proxy-server"));
  command_line->AppendSwitch(CefString("winhttp-proxy-resolver"));
  command_line->AppendSwitch(CefString("no-sandbox"));
}

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();
  
  std::cout << "SimpleApp::OnContextInitialized" << std::endl;
}
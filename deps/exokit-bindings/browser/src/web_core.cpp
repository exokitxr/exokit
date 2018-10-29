// Ŭnicode please
#include "web_core.h"
#include "browser_client.h"
#include "render_handler.h"

#include <cassert>
#include <include/cef_app.h>

WebCoreManager::WebCoreManager()
{

}

WebCoreManager::~WebCoreManager()
{

}

bool WebCoreManager::setUp(int *exit_code)
{
	assert(exit_code != nullptr);

	CefMainArgs args;
	*exit_code = CefExecuteProcess(args, nullptr, nullptr);;
	if (*exit_code >= 0) { 
		return false;
	}

	CefSettings settings;
	bool result = CefInitialize(args, settings, nullptr, nullptr);
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

std::weak_ptr<WebCore> WebCoreManager::createBrowser(const std::string &url)
{
	auto web_core = std::make_shared<WebCore>(url);
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

WebCore::WebCore(const std::string &url)
	: mouse_x_(0), mouse_y_(0)
{
	render_handler_ = new RenderHandler();
	render_handler_->init();
	// initial size
	render_handler_->resize(128, 128);

	CefWindowInfo window_info;
	HWND hwnd = GetConsoleWindow();
	window_info.SetAsWindowless(hwnd, true);

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
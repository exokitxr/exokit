#ifndef _WEB_CORE_H_
#define _WEB_CORE_H_

#include <include/cef_client.h>
#include <include/cef_app.h>
#include <include/wrapper/cef_helpers.h>

#include "render_handler.h"

#include <string>
#include <vector>
#include <functional>

class RenderHandler;
class BrowserClient;

class WebCore {
public:
	WebCore(const std::string &url, RenderHandler::OnPaintFn onPaint);
	~WebCore();

	void reshape(int w, int h);

	void mouseMove(int x, int y);
	void mouseClick(CefBrowserHost::MouseButtonType btn, bool mouse_up);
	void keyPress(int key, bool pressed);

	RenderHandler* render_handler() const { return render_handler_; }

private:
	int mouse_x_;
	int mouse_y_;

	CefRefPtr<CefBrowser> browser_;
	CefRefPtr<BrowserClient> client_;

	RenderHandler* render_handler_;
};

class WebCoreManager {
public:
	WebCoreManager();
	~WebCoreManager();

	bool setUp(int *exit_code);
	bool shutDown();

	void update();

	std::weak_ptr<WebCore> createBrowser(const std::string &url, RenderHandler::OnPaintFn onPaint);
	void removeBrowser(std::weak_ptr<WebCore> web_core);

private:
	std::vector<std::shared_ptr<WebCore>> browsers_;
};

class SimpleApp : public CefApp, public CefBrowserProcessHandler {
 public:
  SimpleApp();

  // CefApp methods:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  virtual void OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line) override;
  
  // CefBrowserProcessHandler methods:
  virtual void OnContextInitialized() override;

 private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif
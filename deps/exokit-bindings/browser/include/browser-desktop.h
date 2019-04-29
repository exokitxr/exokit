#ifndef _BROWSER_DESKTOP_H_
#define _BROWSER_DESKTOP_H_

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <functional>

#include <webgl.h>
#include <browser-common.h>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

// SimpleApp

class SimpleApp : public CefApp, public CefBrowserProcessHandler {
public:
  SimpleApp(const std::string &dataPath);

  // CefApp methods:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  virtual void OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line) override;
  
  // CefBrowserProcessHandler methods:
  virtual void OnContextInitialized() override;

protected:
  std::string dataPath;

private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleApp);
};

// LoadHandler

class LoadHandler : public CefLoadHandler {
public:
	LoadHandler(std::function<void()> onLoadStart, std::function<void()> onLoadEnd, std::function<void(int, const std::string &, const std::string &)> onLoadError);
  ~LoadHandler();

	// CefRenderHandler interface
public:
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) override;
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString &failedUrl) override;

	// CefBase interface
private:
  IMPLEMENT_REFCOUNTING(LoadHandler);

private:
  std::function<void()> onLoadStart;
  std::function<void()> onLoadEnd;
  std::function<void(int, const std::string &, const std::string &)> onLoadError;
};

// DisplayHandler

class DisplayHandler : public CefDisplayHandler {
public:
	DisplayHandler(std::function<void(const std::string &, const std::string &, int)> onConsole, std::function<void(const std::string &)> onMessage);
  ~DisplayHandler();

	// CefRenderHandler interface
public:
	virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level, const CefString &message, const CefString &source, int line) override;

	// CefBase interface
private:
  IMPLEMENT_REFCOUNTING(DisplayHandler);

private:
  std::function<void(const std::string &, const std::string &, int)> onConsole;
  std::function<void(const std::string &)> onMessage;
};

// RenderHandler

class RenderHandler : public CefRenderHandler {
public:
  typedef std::function<void(const RectList &, const void *, int, int)> OnPaintFn;
  
	RenderHandler(OnPaintFn onPaint, int width, int height);
  ~RenderHandler();

	// void resize(int w, int h);

	// CefRenderHandler interface
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;
	virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) override;

// protected:
  int width;
  int height;
  bool resized;
  std::function<void(const RectList &, const void *, int, int)> onPaint;

	// CefBase interface
private:
  IMPLEMENT_REFCOUNTING(RenderHandler);
};

// BrowserClient

class BrowserClient : public CefClient {
public:
	BrowserClient(LoadHandler *loadHandler, DisplayHandler *displayHandler, RenderHandler *renderHandler);
  ~BrowserClient();
  
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override {
		return m_loadHandler;
	}
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override {
    return m_displayHandler;
  }
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override {
		return m_renderHandler;
	}

	CefRefPtr<LoadHandler> m_loadHandler;
	CefRefPtr<DisplayHandler> m_displayHandler;
	CefRefPtr<RenderHandler> m_renderHandler;

private:
	IMPLEMENT_REFCOUNTING(BrowserClient);
};

}

#endif

#ifndef _BROWSER_DESKTOP_H_
#define _BROWSER_DESKTOP_H_

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <functional>

#include <webgl.h>

#include <include/capi/cef_app_capi.h>
#include <include/cef_client.h>
#include <include/cef_app.h>
#include <include/cef_load_handler.h>
#include <include/cef_render_handler.h>

#include <libcef_dll/cpptoc/app_cpptoc.h>
#include <libcef_dll/cpptoc/client_cpptoc.h>
#include <libcef_dll/ctocpp/request_context_ctocpp.h>
#include <libcef_dll/ctocpp/browser_ctocpp.h>

using namespace std;
using namespace v8;
using namespace node;

typedef CefRefPtr<CefBrowser> EmbeddedBrowser;

namespace browser {
  
bool initializeEmbedded(const std::string &dataPath);
EmbeddedBrowser createEmbedded(
  const std::string &url,
  WebGLRenderingContext *gl,
  NATIVEwindow *window,
  GLuint tex,
  int width,
  int height,
  int *textureWidth,
  int *textureHeight,
  std::function<void()> onloadstart,
  std::function<void(const std::string &)> onloadend,
  std::function<void(int, const std::string &, const std::string &)> onloaderror,
  std::function<void(const std::string &, const std::string &, int)> onconsole,
  std::function<void(const std::string &)> onmessage
);
void destroyEmbedded(EmbeddedBrowser browser_);
void embeddedDoMessageLoopWork();
int getEmbeddedWidth(EmbeddedBrowser browser_);
void setEmbeddedWidth(EmbeddedBrowser browser_, int width);
int getEmbeddedHeight(EmbeddedBrowser browser_);
void setEmbeddedHeight(EmbeddedBrowser browser_, int height);
void embeddedGoBack(EmbeddedBrowser browser_);
void embeddedGoForward(EmbeddedBrowser browser_);
void embeddedReload(EmbeddedBrowser browser_);
void embeddedMouseMove(EmbeddedBrowser browser_, int x, int y);
void embeddedMouseDown(EmbeddedBrowser browser_, int x, int y, int button);
void embeddedMouseUp(EmbeddedBrowser browser_, int x, int y, int button);
void embeddedMouseWheel(EmbeddedBrowser browser_, int x, int y, int deltaX, int deltaY);
void embeddedKeyDown(EmbeddedBrowser browser_, int key, int modifiers);
void embeddedKeyUp(EmbeddedBrowser browser_, int key, int modifiers);
void embeddedKeyPress(EmbeddedBrowser browser_, int key, int wkey, EmbeddedKeyModifiers modifiers);
void embeddedRunJs(EmbeddedBrowser browser_, const std::string &jsString, const std::string &scriptUrl, int startLine);

CefBrowserHost::MouseButtonType GetMouseButton(int button);

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

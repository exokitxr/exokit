#ifndef _BROWSER_H_
#define _BROWSER_H_

#include <webgl.h>

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>

#include <chrono>
// #include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <functional>

#include <include/cef_client.h>
#include <include/cef_app.h>
#include <include/cef_render_handler.h>
#include <include/wrapper/cef_helpers.h>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

// SimpleApp

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

// RenderHandler

class RenderHandler : public CefRenderHandler {
public:
  typedef std::function<void(const RectList &, const void *, int, int)> OnPaintFn;
  
public:
	RenderHandler(OnPaintFn onPaint);
  ~RenderHandler();

	void resize(int w, int h);

	// CefRenderHandler interface
public:
	bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect);
	void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height);

	// CefBase interface
private:
  IMPLEMENT_REFCOUNTING(RenderHandler);

private:
  int width;
	int height;
  std::function<void(const RectList &, const void *, int, int)> onPaint;
};

// BrowserClient

class BrowserClient : public CefClient {
public:
	BrowserClient(RenderHandler *renderHandler);
  ~BrowserClient();

	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() {
		return m_renderHandler;
	}

	CefRefPtr<CefRenderHandler> m_renderHandler;

private:
	IMPLEMENT_REFCOUNTING(BrowserClient);
};

// Browser

class Browser : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  Browser(WebGLRenderingContext *gl, int width, int height, const std::string &url);
  ~Browser();

  static NAN_METHOD(New);
  static NAN_METHOD(Update);
  static NAN_GETTER(TextureGetter);
  void reshape(int w, int h);
protected:
  GLuint tex;
  bool initialized;
  std::unique_ptr<RenderHandler> render_handler_;
  std::unique_ptr<BrowserClient> client_;
  CefRefPtr<CefBrowser> browser_;
};

// helpers

void QueueOnBrowserThread(std::function<void()> fn);

void RunOnMainThread(std::function<void()> fn);
void MainThreadAsync(uv_async_t *handle);

// variables

extern bool cefInitialized;
extern std::thread browserThread;

extern uv_sem_t constructSem;
extern uv_sem_t paintSem;

extern std::mutex browserThreadFnMutex;
extern std::deque<std::function<void()>> browserThreadFns;

extern uv_async_t mainThreadAsync;
extern std::mutex mainThreadFnMutex;
extern std::deque<std::function<void()>> mainThreadFns;

}

#endif

#ifndef _BROWSER_COMMON_H_
#define _BROWSER_COMMON_H_

#include <webgl.h>

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>

#include <chrono>
#include <deque>
#include <thread>
#include <mutex>
#include <functional>

#ifndef LUMIN

#include <include/capi/cef_app_capi.h>
#include <include/cef_client.h>
#include <include/cef_app.h>
#include <include/cef_load_handler.h>
#include <include/cef_render_handler.h>

#include <libcef_dll/cpptoc/app_cpptoc.h>
#include <libcef_dll/cpptoc/client_cpptoc.h>
#include <libcef_dll/ctocpp/request_context_ctocpp.h>
#include <libcef_dll/ctocpp/browser_ctocpp.h>

#else

#include <Servo2D.h>

#endif

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

#ifndef LUMIN
typedef CefRefPtr<CefBrowser> EmbeddedBrowser;
#else
typedef Servo2D *EmbeddedBrowser;
#endif

enum class EmbeddedKeyModifiers {
  SHIFT,
  CTRL,
  ALT,
};

// bindings

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
  std::function<EmbeddedBrowser()> getBrowser,
  std::function<void(EmbeddedBrowser)> setBrowser,
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
void embeddedKeyDown(EmbeddedBrowser browser_, int key, int wkey, int modifiers);
void embeddedKeyUp(EmbeddedBrowser browser_, int key, int wkey, int modifiers);
void embeddedKeyPress(EmbeddedBrowser browser_, int key, int wkey, int modifiers);
void embeddedRunJs(EmbeddedBrowser browser_, const std::string &jsString, const std::string &scriptUrl, int startLine);

// helpers

void QueueOnBrowserThread(std::function<void()> fn);
void QueueOnBrowserThreadFront(std::function<void()> fn);

void RunOnMainThread(std::function<void()> fn);
void QueueOnMainThread(std::function<void()> fn);
void MainThreadAsync(uv_async_t *handle);

// variables

extern bool embeddedInitialized;
extern std::thread browserThread;

extern uv_sem_t constructSem;
extern uv_sem_t mainThreadSem;
extern uv_sem_t browserThreadSem;

extern std::mutex browserThreadFnMutex;
extern std::deque<std::function<void()>> browserThreadFns;

extern uv_async_t mainThreadAsync;
extern std::mutex mainThreadFnMutex;
extern std::deque<std::pair<std::function<void()>, bool>> mainThreadFns;

}

#endif

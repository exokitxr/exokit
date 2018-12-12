#ifndef _BROWSER_ML_H_
#define _BROWSER_ML_H_

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <list>
#include <functional>

#include <magicleap/Servo2D.h>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

typedef Servo2D *EmbeddedBrowser;
  
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
void embeddedKeyPress(EmbeddedBrowser browser_, int key, int wkey, int modifiers);
void embeddedRunJs(EmbeddedBrowser browser_, const std::string &jsString, const std::string &scriptUrl, int startLine);

extern std::list<EmbeddedBrowser> browsers;

}

#endif

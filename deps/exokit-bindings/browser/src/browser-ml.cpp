#ifdef LUMIN

#include <browser-ml.h>

#include <exout>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

bool initializeEmbedded(const std::string &dataPath, const string &frameworkPath) {
  Servo2D::init();
  return true;
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
  exout << "createEmbedded 0 " << (void *)window << std::endl;
  
  if (width == 0) {
    width = 1280;
  }
  if (height == 0) {
    height = 1024;
  }
  
  *textureWidth = width;
  *textureHeight = height;

  EmbeddedBrowser browser_ = getBrowser();
  
  exout << "createEmbedded 1 " << (void *)window << std::endl;
  
  {
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
      exout << "createEmbedded error 1 " << error << std::endl;
    }
  }
  /* windowsystem::SetCurrentWindowContext(window);
  {
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
      exout << "createEmbedded error 2 " << error << std::endl;
    }
  } */

  {
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
      exout << "createEmbedded error 5 " << error << std::endl;
    }
  }

  if (!browser_) {
    browser_.reset(new Servo2D());
    {
      EGLint error = eglGetError();
      if (error != EGL_SUCCESS) {
        exout << "createEmbedded error 6 " << error << std::endl;
      }
    }
    browser_->init(url, window, tex, width, height, onloadstart, onloadend, onloaderror, onconsole, onmessage);
    browsers.push_back(browser_);
    
    {
      EGLint error = eglGetError();
      if (error != EGL_SUCCESS) {
        exout << "createEmbedded error 7 " << error << std::endl;
      }
    }
    
    /* if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D)) {
      glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D));
    } else {
      glBindTexture(GL_TEXTURE_2D, 0);
    } */
  } else {
    browser_->navigate(url);
  }
  
  exout << "createEmbedded 8 " << std::endl;
  
  return browser_;
}
void destroyEmbedded(EmbeddedBrowser browser_) {
  browser_->deInit();
  browsers.erase(std::find(browsers.begin(), browsers.end(), browser_));
}
void embeddedUpdate() {
  // nothing
}
void embeddedDoMessageLoopWork() {
  // exout << "do message loop work 1 " << browsers.size() << std::endl;

  for (EmbeddedBrowser browser_ : browsers) {
    heartbeat_servo(browser_->getInstance());
    /* for (int i = 0; i < 100; i++) {
      heartbeat_servo(browser_->getInstance());
    }
    browser_->flushTexture(); */
  }
  
  // exout << "do message loop work 2 " << browsers.size() << std::endl;
}
std::pair<int, int> getEmbeddedSize(EmbeddedBrowser browser_) {
  return std::pair<int, int>(browser_->getWidth(), browser_->getHeight());
}
void setEmbeddedSize(EmbeddedBrowser browser_, int width, int height) {
  browser_->setWidth(width);
  browser_->setHeight(height);
}
int getEmbeddedWidth(EmbeddedBrowser browser_) {
  return browser_->getWidth();
}
void setEmbeddedWidth(EmbeddedBrowser browser_, int width) {
  browser_->setWidth(width);
}
int getEmbeddedHeight(EmbeddedBrowser browser_) {
  return browser_->getHeight();
}
void setEmbeddedHeight(EmbeddedBrowser browser_, int height) {
  browser_->setHeight(height);
}
float getEmbeddedScale(EmbeddedBrowser browser_) {
  return 1;
}
void setEmbeddedScale(EmbeddedBrowser browser_, float scale) {
  // nothing
}
void embeddedGoBack(EmbeddedBrowser browser_) {
  traverse_servo(browser_->getInstance(), -1);
}
void embeddedGoForward(EmbeddedBrowser browser_) {
  traverse_servo(browser_->getInstance(), 1);
}
void embeddedReload(EmbeddedBrowser browser_) {
  traverse_servo(browser_->getInstance(), 0);
}
void embeddedMouseMove(EmbeddedBrowser browser_, int x, int y) {
  move_servo(browser_->getInstance(), x, y);
}
void embeddedMouseDown(EmbeddedBrowser browser_, int x, int y, int button) {
  trigger_servo(browser_->getInstance(), x, y, true);
}
void embeddedMouseUp(EmbeddedBrowser browser_, int x, int y, int button) {
  trigger_servo(browser_->getInstance(), x, y, false);
}
void embeddedMouseWheel(EmbeddedBrowser browser_, int x, int y, int deltaX, int deltaY) {
  // nothing
}
void embeddedKeyDown(EmbeddedBrowser browser_, int key, int wkey, int modifiers) {
  uint32_t keycode = (uint32_t)key;
  bool shift = (bool)(modifiers & (int)EmbeddedKeyModifiers::SHIFT);
  bool ctrl = (bool)(modifiers & (int)EmbeddedKeyModifiers::CTRL);
  bool alt = (bool)(modifiers & (int)EmbeddedKeyModifiers::ALT);
  bool logo = false;
  bool down = true;

  keyboard_servo(browser_->getInstance(), keycode, shift, ctrl, alt, logo, down);
}
void embeddedKeyUp(EmbeddedBrowser browser_, int key, int wkey, int modifiers) {
  uint32_t keycode = (uint32_t)key;
  bool shift = (bool)(modifiers & (int)EmbeddedKeyModifiers::SHIFT);
  bool ctrl = (bool)(modifiers & (int)EmbeddedKeyModifiers::CTRL);
  bool alt = (bool)(modifiers & (int)EmbeddedKeyModifiers::ALT);
  bool logo = false;
  bool down = false;

  keyboard_servo(browser_->getInstance(), keycode, shift, ctrl, alt, logo, down);
}
void embeddedKeyPress(EmbeddedBrowser browser_, int key, int wkey, int modifiers) {
  // nothing; servo handles keypress events internally
}
void embeddedRunJs(EmbeddedBrowser browser_, const std::string &jsString, const std::string &scriptUrl, int startLine) {
  executejs_servo(browser_->getInstance(), (const uint8_t *)jsString.c_str(), jsString.length());
}

std::list<EmbeddedBrowser> browsers;

}

#endif

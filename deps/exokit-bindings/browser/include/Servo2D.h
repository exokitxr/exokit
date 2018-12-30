/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef _SERVO_2D_H_
#define _SERVO_2D_H_

#include <webgl.h>
#include <ml_logging.h>
#include <dlfcn.h>
// #include <string.h>

namespace browser {

// forward declarations

class Servo2D;
typedef struct Opaque ServoInstance;
typedef void (*MLLogger)(Servo2D *app, MLLogLevel lvl, char *msg, size_t size);
typedef void (*MLHistoryUpdate)(Servo2D *app, bool canGoBack, char* url, bool canGoForward);
typedef void (*MLPresentUpdate)(Servo2D *app);

/**
 * Servo2D Landscape Application
 */
class Servo2D {
public:

  /**
   * Constructs the Landscape Application.
   */
  Servo2D();

  /**
   * Destroys the Landscape Application.
   */
  ~Servo2D();

  /**
   * Disallows the copy constructor.
   */
  Servo2D(const Servo2D&) = delete;

  /**
   * Disallows the move constructor.
   */
  Servo2D(Servo2D&&) = delete;

  /**
   * Disallows the copy assignment operator.
   */
  Servo2D& operator=(const Servo2D&) = delete;

  /**
   * Disallows the move assignment operator.
   */
  Servo2D& operator=(Servo2D&&) = delete;

// protected:
  /**
   * Initializes the Landscape Application.
   * @return - 0 on success, error code on failure.
   */
  int init(
    const std::string &url,
    NATIVEwindow *window,
    GLuint tex,
    int width,
    int height,
    std::function<void()> onloadstart,
    std::function<void(const std::string &)> onloadend,
    std::function<void(int, const std::string &, const std::string &)> onloaderror,
    std::function<void(const std::string &, const std::string &, int)> onconsole,
    std::function<void(const std::string &)> onmessage
  );

  /**
   * Deinitializes the Landscape Application.
   * @return - 0 on success, error code on failure.
   */
  int deInit();
  
  void navigate(const std::string &url);
  
  int getWidth() const;
  void setWidth(int width);
  int getHeight() const;
  void setHeight(int height);

  ServoInstance *getInstance() const;
  void flushTexture() const;
  static void present(Servo2D *app);
  
  static void init();

private:
  std::string url;
  GLuint tex;
  int width;
  int height;
  int x;
  int y;
  std::function<void()> onloadstart;
  std::function<void(const std::string &)> onloadend;
  std::function<void(int, const std::string &, const std::string &)> onloaderror;
  std::function<void(const std::string &, const std::string &, int)> onconsole;
  std::function<void(const std::string &)> onmessage;
  
  GLuint fboOutCache;
  
  ServoInstance *servo_;
  EGLDisplay display;
  EGLSurface surface;
};

};

// externs

extern "C" browser::ServoInstance *(*init_servo)(EGLContext, EGLSurface, EGLDisplay,
                                     browser::Servo2D*, browser::MLLogger, browser::MLHistoryUpdate, browser::MLPresentUpdate,
                                     const char* url, int width, int height, float hidpi);
extern "C" void (*heartbeat_servo)(browser::ServoInstance*);
extern "C" void (*trigger_servo)(browser::ServoInstance*, float x, float y, bool down);
extern "C" void (*move_servo)(browser::ServoInstance*, float x, float y);
extern "C" void (*traverse_servo)(browser::ServoInstance*, int delta);
extern "C" void (*navigate_servo)(browser::ServoInstance*, const char* text);
extern "C" void (*discard_servo)(browser::ServoInstance*);

#endif
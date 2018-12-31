/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifdef LUMIN

#include <Servo2D.h>

#include <iostream>

namespace browser {

// A function which calls the ML logger, suitable for passing into Servo
void logger(Servo2D *app, MLLogLevel lvl, char *msg, size_t size) {
  std::string jsString(msg, size);
  std::string scriptUrl;
  int startLine = 0;
  app->onconsole(jsString, scriptUrl, startLine);
  /* if (MLLoggingLogLevelIsEnabled(lvl)) {
    MLLoggingLog(lvl, ML_DEFAULT_LOG_TAG, msg);
  } */
}

// A function which updates the history ui, suitable for passing into Servo
void history(Servo2D *app, bool canGoBack, char* url, bool canGoForward) {
  // app->updateHistory(canGoBack, url, canGoForward);
}

// Create a Servo2D instance
Servo2D::Servo2D() : servo_(nullptr) {
  ML_LOG(Debug, "Servo2D Constructor.");
}

// Destroy a Servo 2D instance
Servo2D::~Servo2D() {
  ML_LOG(Debug, "Servo2D Destructor.");
}

// Initialize a Servo instance
int Servo2D::init(
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
) {
  std::cout << "Servo2D Initializing 1" << tex << " " << width << " " << height << std::endl;
  {
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
      std::cout << "Servo2D error 1 " << error << std::endl;
    }
  }

  this->url = url;
  this->tex = tex;
  this->width = width;
  this->height = height;
  this->onloadstart = onloadstart;
  this->onloadend = onloadend;
  this->onloaderror = onloaderror;
  this->onconsole = onconsole;
  this->onmessage = onmessage;

  // Get the EGL context, surface and display.
  EGLContext context = windowsystem::GetGLContext(window);
  this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLConfig eglConfig;
  EGLint eglConfigAttribs[] = {
    EGL_RED_SIZE, 5,
    EGL_GREEN_SIZE, 6,
    EGL_BLUE_SIZE,  5,
    EGL_ALPHA_SIZE, 0,
    /* EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8, */
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    // EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT|EGL_PBUFFER_BIT,
    EGL_NONE,
  };
  int numConfigs;
  eglChooseConfig(this->display, eglConfigAttribs, &eglConfig, 1, &numConfigs);
  std::cout << "Servo2D 2 " << numConfigs << std::endl;
  {
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
      std::cout << "Servo2D error 3 " << error << std::endl;
    }
  }
  EGLint surfaceAttribs[] = {
    EGL_WIDTH, width,
    EGL_HEIGHT, height,
    EGL_NONE
  };
  this->surface = eglCreatePbufferSurface(this->display, eglConfig, surfaceAttribs);
  std::cout << "Servo2D init 4 " << (void *)(this->surface) << std::endl;
  {
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
      std::cout << "Servo2D error 5 " << error << std::endl;
    }
  }
  /* if (!this->surface) {
    exit(0);
  } */
  
  {
    eglMakeCurrent(this->display, this->surface, this->surface, context);
    {
      EGLint error = eglGetError();
      if (error != EGL_SUCCESS) {
        std::cout << "Servo2D error 6 " << error << std::endl;
      }
    }
  }
  // glClearColor(1.0, 0.0, 0.0, 1.0);
  // glClear(GL_COLOR_BUFFER_BIT);
  {
    {
      GLenum error = glGetError();
      if (error) {
        std::cout << "Servo2D error 7 " << error << std::endl;
      }
    }
    glBindTexture(GL_TEXTURE_2D, tex);
    {
      GLenum error = glGetError();
      if (error) {
        std::cout << "Servo2D error 8 " << error << std::endl;
      }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    {
      GLenum error = glGetError();
      if (error) {
        std::cout << "Servo2D error 9 " << error << std::endl;
      }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  {
    glGenFramebuffers(1, &this->fboOutCache);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->fboOutCache);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->tex, 0);

    {
      GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
      if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "incomplete out framebuffer " << status << std::endl;
      }
    }
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }

  // Hook into servo
  servo_ = init_servo(context, this->surface, this->display, this, logger, history, Servo2D::present, url.c_str(), width, height, 1.0);
  std::cout << "Servo2D Initializing 10 " << url << " " << width << " " << height << " " << (void *)servo_ << std::endl;
  if (!servo_) {
    ML_LOG(Error, "Servo2D Failed to init servo instance");
    abort();
    return 1;
  }
  
  onloadstart();
  onloadend(url);

  return 0;
}

int Servo2D::deInit() {
  ML_LOG(Debug, "Servo2D Deinitializing.");
  discard_servo(servo_);
  servo_ = nullptr;
  return 0;
}

void Servo2D::navigate(const std::string &url) {
  navigate_servo(servo_, url.c_str());
}

int Servo2D::getWidth() const {
  return width;
}
void Servo2D::setWidth(int width) {
  this->width = width;
}
int Servo2D::getHeight() const {
  return height;
}
void Servo2D::setHeight(int height) {
  this->height = height;
}

ServoInstance *Servo2D::getInstance() const {
  return servo_;
}

void Servo2D::flushTexture() const {
  // std::cout << "flush texture start " << drawFboId << " " << readFboId << " " << (void *)(this->surface) << " " << this->fboOutCache << " " << this->tex << std::endl;
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  {
    GLuint error = glGetError();
    if (error) {
      std::cout << "flush texture error 0 " << error << std::endl;
    }
  }
  {
    GLenum status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "flush incomplete read framebuffer " << status << std::endl;
    }
  }
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->fboOutCache);
  {
    GLuint error = glGetError();
    if (error) {
      std::cout << "flush texture error 1 " << error << std::endl;
    }
  }
  {
    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "flush incomplete draw framebuffer " << status << std::endl;
    }
  }

  glBlitFramebuffer(
    0, this->height,
    this->width, 0,
    0, 0,
    this->width, this->height,
    GL_COLOR_BUFFER_BIT,
    GL_NEAREST
  );
  {
    GLuint error = glGetError();
    if (error) {
      std::cout << "flush texture error 2 " << error << " " << std::endl;
    }
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // std::cout << "flush texture end" << std::endl;
}

void Servo2D::present(Servo2D *app) {
  app->flushTexture();
}

void Servo2D::init() {
  void *libmlservo = dlopen("/package/bin/libmlservo.so", RTLD_LAZY);
  if (!libmlservo) {
    std::cout << "failed to dlopen libmlservo.so" << std::endl;
  }
  
  init_servo = (ServoInstance *(*)(EGLContext, EGLSurface, EGLDisplay,
    Servo2D*, MLLogger, MLHistoryUpdate, MLPresentUpdate,
    const char* url, int width, int height, float hidpi))dlsym(libmlservo, "init_servo");
  heartbeat_servo = (void (*)(ServoInstance*))dlsym(libmlservo, "heartbeat_servo");
  trigger_servo = (void (*)(ServoInstance*, float x, float y, bool down))dlsym(libmlservo, "trigger_servo");
  move_servo = (void (*)(ServoInstance*, float x, float y))dlsym(libmlservo, "move_servo");
  keyboard_servo = (void (*)(ServoInstance*, uint32_t keycode, bool shift, bool ctrl, bool alt, bool logo, bool down))dlsym(libmlservo, "keyboard_servo");
  executejs_servo = (void (*)(ServoInstance*, const uint8_t *data, size_t length))dlsym(libmlservo, "executejs_servo");
  traverse_servo = (void (*)(ServoInstance*, int delta))dlsym(libmlservo, "traverse_servo");
  navigate_servo = (void (*)(ServoInstance*, const char* text))dlsym(libmlservo, "navigate_servo");
  discard_servo = (void (*)(ServoInstance*))dlsym(libmlservo, "discard_servo");
  if (!init_servo || !heartbeat_servo || !trigger_servo || !move_servo || !keyboard_servo || !traverse_servo || !navigate_servo || !discard_servo) {
    std::cout << "failed to dlsym libmlservo.so symbols " << (void *)init_servo << " " << (void *)heartbeat_servo << " " << (void *)trigger_servo << " " << (void *)move_servo << " " << (void *)traverse_servo << " " << (void *)navigate_servo << " " << (void *)discard_servo << std::endl;
  }
}

// GL extra hacks

GLvoid glTexStorage2DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
  return glTexStorage2D(target, levels, internalformat, width, height);
}
GLvoid glFramebufferTexture2DMultisampleEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples) {
  return glFramebufferTexture2D(target, attachment, textarget, texture, level);
}
GLvoid glRenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
  return glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}
GLvoid *glMapBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
  return glMapBufferRange(target, offset, length, access);
}
GLvoid glFlushMappedBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length) {
  return glFlushMappedBufferRange(target, offset, length);
}

};

// externs

browser::ServoInstance *(*init_servo)(EGLContext, EGLSurface, EGLDisplay,
  browser::Servo2D*, browser::MLLogger, browser::MLHistoryUpdate, browser::MLPresentUpdate,
  const char*url, int width, int height, float hidpi) = nullptr;
void (*heartbeat_servo)(browser::ServoInstance*) = nullptr;
void (*trigger_servo)(browser::ServoInstance*, float x, float y, bool down) = nullptr;
void (*move_servo)(browser::ServoInstance*, float x, float y) = nullptr;
void (*keyboard_servo)(browser::ServoInstance*, uint32_t keycode, bool shift, bool ctrl, bool alt, bool logo, bool down) = nullptr;
void (*executejs_servo)(browser::ServoInstance*, const uint8_t *data, size_t length) = nullptr;
void (*traverse_servo)(browser::ServoInstance*, int delta) = nullptr;
void (*navigate_servo)(browser::ServoInstance*, const char* text) = nullptr;
void (*discard_servo)(browser::ServoInstance*) = nullptr;

#endif
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifdef LUMIN

#include <Servo2D.h>

#include <iostream>

// A function which calls the ML logger, suitable for passing into Servo
void logger(MLLogLevel lvl, char* msg) {
  if (MLLoggingLogLevelIsEnabled(lvl)) {
    MLLoggingLog(lvl, ML_DEFAULT_LOG_TAG, msg);
  }
}

// A function which updates the history ui, suitable for passing into Servo
void history(Servo2D* app, bool canGoBack, char* url, bool canGoForward) {
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
    if (error) {
      std::cout << "Servo2D error 1 " << error << " " << std::endl;
    }
  }
  {
    EGLint error = eglGetError();
    if (error) {
      std::cout << "Servo2D error 2 " << error << std::endl;
    }
  }
  int maxTextureSize = -1;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
  {
    EGLint error = eglGetError();
    if (error) {
      std::cout << "Servo2D error 3 " << error << " " << std::endl;
    }
  }
  {
    EGLint error = eglGetError();
    if (error) {
      std::cout << "Servo2D error 4 " << error << std::endl;
    }
  }
  std::cout << "Servo2D Initializing 2 " << maxTextureSize << std::endl;

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
  EGLContext ctx = windowsystem::GetGLContext(window);
  this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  EGLint attribs[] = {
    EGL_WIDTH, width,
    EGL_HEIGHT, height,
    EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
    EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
    EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
    EGL_NONE
  };
  EGLConfig eglConfig = nullptr;
  this->surface = eglCreatePbufferSurface(this->display, eglConfig, attribs);
  
  std::cout << "Servo2D Initializing 3 " << maxTextureSize << std::endl;

  // Hook into servo
  servo_ = init_servo(ctx, this->surface, this->display, this, logger, history, url.c_str(), width, height, 1.0);
  std::cout << "Servo2D Initializing 4 " << (void *)servo_ << std::endl;
  if (!servo_) {
    ML_LOG(Error, "Servo2D Failed to init servo instance");
    abort();
    return 1;
  }
  
  std::cout << "Servo2D Initializing 5 " << std::endl;

  return 0;
}

int Servo2D::deInit() {
  ML_LOG(Debug, "Servo2D Deinitializing.");
  discard_servo(servo_);
  servo_ = nullptr;
  return 0;
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
  glBindTexture(GL_TEXTURE_2D, tex);
  
  eglBindTexImage(display, surface, EGL_BACK_BUFFER);
  
  // XXX copy textures here
  
  eglReleaseTexImage(display, surface, EGL_BACK_BUFFER);
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

#endif

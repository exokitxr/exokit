#ifndef _EGL_BINDINGS_H_
#define _EGL_BINDINGS_H_

#include <string>
#include <sstream>
#include <map>

#include <v8.h>
#include <nan.h>
#include <defines.h>

#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#endif
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR 0
#define EGL_CONTEXT_RELEASE_BEHAVIOR_KHR  0x2097
#define EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR 0x2098

using namespace v8;

class WebGLRenderingContext;
typedef unsigned int GLuint;

class NATIVEwindow {
public:
  EGLDisplay display;
  EGLContext context;
  unsigned int width;
  unsigned int height;
};
#define windowsystem egl

namespace egl {
  NATIVEwindow *CreateNativeWindow(unsigned int width, unsigned int height, bool visible);
  void GetScreenSize(int *width, int *height);
  void GetWindowSize(NATIVEwindow *window, int *width, int *height);
  void GetFramebufferSize(NATIVEwindow *window, int *width, int *height);
  EGLContext GetGLContext(NATIVEwindow *window);
  NATIVEwindow *GetCurrentWindowContext();
  void SetCurrentWindowContext(NATIVEwindow *window);
  void ReadPixels(WebGLRenderingContext *gl, unsigned int fbo, int x, int y, int width, int height, unsigned int format, unsigned int type, unsigned char *data);
}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif

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
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
/* #include <GLES3/gl3.h>
#include <GLES3/gl3ext.h> */

// #include <webgl.h>

using namespace v8;

class NATIVEwindow {
public:
  EGLDisplay display;
  EGLContext context;
  unsigned int width;
  unsigned int height;
};
#define windowsystem egl

namespace egl {
  void GetWindowSize(NATIVEwindow *window, int *width, int *height);
  void *GetGLContext(NATIVEwindow *window);
  void SetCurrentWindowContext(NATIVEwindow *window);
}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif
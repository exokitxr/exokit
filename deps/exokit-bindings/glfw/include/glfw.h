#ifndef _GLFW_BINDINGS_H_
#define _GLFW_BINDINGS_H_

#include <string>
#include <sstream>
#include <map>

#include <v8.h>
#include <nan.h>
#include <defines.h>

#include <GL/glew.h>

#define GLFW_NO_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>

// #include <webgl.h>

using namespace v8;

class WebGLRenderingContext;

typedef GLFWwindow NATIVEwindow;
#define windowsystem glfw

namespace glfw {
  void GetWindowSize(NATIVEwindow *window, int *width, int *height);
  void *GetGLContext(NATIVEwindow *window);
  void SetCurrentWindowContext(NATIVEwindow *window);
  void ReadPixels(WebGLRenderingContext *gl, unsigned int fbo, int x, int y, int width, int height, unsigned int format, unsigned int type, unsigned char *data);
}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif
#ifndef _GLFW_BINDINGS_H_
#define _GLFW_BINDINGS_H_

#include <string>
#include <sstream>
#include <map>

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>

#include <GL/glew.h>

#define GLFW_NO_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>

// #include <webgl.h>

using namespace v8;

typedef GLFWwindow NATIVEwindow;
#define windowsystem glfw

namespace glfw {
  void GetWindowSize(NATIVEwindow *window, int *width, int *height);
  void *GetGLContext(NATIVEwindow *window);
  void SetCurrentWindowContext(NATIVEwindow *window);
}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif
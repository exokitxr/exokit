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

using namespace v8;

namespace glfw {
  void SetCurrentWindowContext(GLFWwindow *window);

  class WindowState {
    public:
      WindowState() {}
      WindowState(GLuint userVao, GLuint systemVao) : userVao(userVao), systemVao(systemVao) {}
      WindowState(const WindowState &windowState) : userVao(windowState.userVao), systemVao(windowState.systemVao) {}
      WindowState(WindowState &windowState) : userVao(windowState.userVao), systemVao(windowState.systemVao) {}
      ~WindowState() {}

      GLuint userVao;
      GLuint systemVao;
  };
}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif
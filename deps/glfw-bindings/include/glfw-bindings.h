#ifndef _GLFW_BINDINGS_H_
#define _GLFW_BINDINGS_H_

#include <string>
#include <sstream>

#include <v8.h>
#include <nan/nan.h>
#include <defines.h>

#include <GL/glew.h>

#define GLFW_NO_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>

using namespace v8;
// using namespace node;

namespace glfw {
  Local<Array> pointerToArray(void *ptr);
  void *arrayToPointer(Local<Array> array);
}

// Local<Object> makeGlfw();
Local<Object> makeWindow();

#endif
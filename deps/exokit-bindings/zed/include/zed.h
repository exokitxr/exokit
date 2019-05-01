#ifndef _ZED_H_
#define _ZED_H_

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>

#include <chrono>

#include <sl/Camera.hpp>
#include <webgl.h>
#include <cuda_gl_interop.h>

using namespace v8;
using namespace node;

namespace zed {

class Zed : public ObjectWrap {
public:
  static Local<Object> Initialize(Isolate *isolate);

protected:
  Zed();
  ~Zed();

  static NAN_METHOD(New);
  static NAN_METHOD(RequestPresent);
  static NAN_METHOD(ExitPresent);
  static NAN_METHOD(WaitGetPoses);

  sl::Camera camera;
  sl::Mesh mesh;
  std::chrono::high_resolution_clock::time_point ts_last;
  GLuint tex;
  NATIVEwindow *window;
  int textureWidth;
  int textureHeight;
  cudaGraphicsResource *pcuImageRes;
  Nan::Persistent<Function> cbFn;
};

}

Local<Object> makeZed();

#endif

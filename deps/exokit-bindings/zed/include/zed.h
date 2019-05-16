#ifndef _ZED_H_
#define _ZED_H_

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>
#include <windowsystem.h>

#include <chrono>
#include <thread>
#include <mutex>

#include <sl/Camera.hpp>
#include <webgl.h>
#include <cuda_gl_interop.h>

using namespace v8;
using namespace node;

namespace zed {

class Zed : public ObjectWrap {
public:
  static Local<Object> Initialize(Isolate *isolate);

// protected:
  Zed();
  ~Zed();

  static NAN_METHOD(New);
  static NAN_METHOD(RequestPresent);
  static NAN_METHOD(ExitPresent);
  static NAN_METHOD(WaitGetPoses);

  void Poll();

  sl::Camera camera;
  sl::Translation position;
  sl::Orientation orientation;
  sl::Mesh mesh;
  sl::Mesh::chunkList chunks;
  std::chrono::high_resolution_clock::time_point ts_last;
  GLuint tex;
  NATIVEwindow *window;
  int textureWidth;
  int textureHeight;
  cudaGraphicsResource *pcuImageRes;
  // Nan::Persistent<Function> cbFn;
  std::mutex mutex;
  std::thread thread;
  uv_async_t *async;
  uv_sem_t reqSem;
  Nan::Persistent<Array> result;
};

}

Local<Object> makeZed();

#endif

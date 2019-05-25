#ifndef _OVR_SESSION_H_
#define _OVR_SESSION_H_

#include <nan.h>
#include <v8.h>
#include <webgl.h>

#include "OVR_CAPI_GL.h"

typedef struct SwapChain {
  ovrTextureSwapChain ColorTextureChain;
  ovrTextureSwapChain DepthTextureChain;
} EyeSwapChain;

namespace oculusvr {
  class OculusVRPosRes;
  void RunResInMainThread(uv_async_t *handle);
}

class OculusVRPosRes {
public:
  OculusVRPosRes(Local<Function> cb);
  ~OculusVRPosRes();

  Nan::Persistent<Function> cb;
};

class OVRSession : public Nan::ObjectWrap
{
public:
  static NAN_MODULE_INIT(Init);

  // Static factory construction method for other node addons to use.
  static v8::Local<v8::Object> NewInstance();

private:
  explicit OVRSession();
  ~OVRSession() = default;

  // Node construction method for new instances.
  static NAN_METHOD(New);
  static NAN_METHOD(CreateSwapChain);
  static NAN_METHOD(ExitPresent);
  static NAN_METHOD(GetControllersInputState);
  static NAN_METHOD(GetPose);
  static NAN_METHOD(Submit);
  static NAN_METHOD(GetRecommendedRenderTargetSize);

  /// Create a singleton reference to a constructor function.
  static inline Nan::Persistent<v8::Function>& constructor()
  {
    static Nan::Persistent<v8::Function> the_constructor;
    return the_constructor;
  }

  static void OculusVRLogCallback(uintptr_t userData, int level, const char *message) {
    if (level >= ovrLogLevel_Error) {
      std::cout << "LibOVR: " << message << std::endl;
    }
  }

  void ResetSwapChain();
  // void EnsureFbos();
  // void AttachFbos();
  void DestroySwapChain();
  void DestroySession();
  void ResetSession();

  /// Reference to wrapped ovrSession instance.
  ovrSession *session;
  ovrHmdDesc hmdDesc;
  GLuint fbo;
  EyeSwapChain swapChain;
  bool swapChainValid;
  ovrPosef eyeRenderPoses[2];
  int swapChainMetrics[2];
  int frameIndex;
  double sensorSampleTime;
  bool hmdMounted;
};

#endif

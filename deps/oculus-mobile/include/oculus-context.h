#ifndef _OCULUS_MOBILE_CONTEXT_H_
#define _OCULUS_MOBILE_CONTEXT_H_

#include <oculus-mobile.h>
#include <oculus-math.h>
#include <egl.h>
#include <webgl.h>

namespace oculusmobile {

extern ovrJava java;

class OculusMobileContext : public ObjectWrap {
public:
  static Local<Function> Initialize();

  static void handleAppCmd(struct android_app *app, int32_t cmd);
  void CreateSwapChain(WebGLRenderingContext *gl, int width, int height);
  void PollEvents(NATIVEwindow *windowHandle);

  static NAN_METHOD(New);
  void Destroy();

  static NAN_METHOD(WaitGetPoses);
  static NAN_METHOD(Submit);
  static NAN_METHOD(GetRecommendedRenderTargetSize);

  OculusMobileContext();
  ~OculusMobileContext();

// protected:
  ovrMobile *ovrState;
  bool running;
  ANativeWindow *androidNativeWindow;
  ovrTextureSwapChain swapChains[2];
  int swapChainMetrics[2];
  int swapChainLength;
  int swapChainIndex;
  bool hasSwapChain;
  GLuint fboId;
  ovrTracking2 tracking;
  long long frameIndex;
	double displayTime;
  int swapInterval;
};

}

#endif

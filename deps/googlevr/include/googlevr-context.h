#ifndef _GOOGLEVR_CONTEXT_H_
#define _GOOGLEVR_CONTEXT_H_

#include <googlevr.h>
#include <googlevr-math.h>
#include <egl.h>
#include <webgl.h>

namespace googlevr {

extern ovrJava java;

class GoogleVrContext : public ObjectWrap {
public:
  static Local<Function> Initialize();

  void RequestPresent();
  static void handleAppCmd(struct android_app *app, int32_t cmd);
  void CreateSwapChain(WebGLRenderingContext *gl, int width, int height);
  void PollEvents(bool wait);

  static NAN_METHOD(New);
  void Destroy();

  static NAN_METHOD(WaitGetPoses);
  static NAN_METHOD(Submit);
  static NAN_METHOD(GetRecommendedRenderTargetSize);

  GoogleVrContext(NATIVEwindow *windowHandle);
  ~GoogleVrContext();

// protected:
  NATIVEwindow *windowHandle;
  // ovrMobile *ovrState;
  bool running;
  ANativeWindow *androidNativeWindow;
  // ovrTextureSwapChain *swapChains[2];
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

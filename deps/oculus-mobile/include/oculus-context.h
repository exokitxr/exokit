#ifndef _OCULUS_MOBILE_CONTEXT_H_
#define _OCULUS_MOBILE_CONTEXT_H_

#include <oculus-mobile.h>
#include <oculus-math.h>
#include <webgl.h>
#include <egl.h>

namespace oculusmobile {

extern ovrJava java;

class OculusMobileContext : public Nan::ObjectWrap {
public:
  static Local<Function> Initialize();

  static void handleAppCmd(struct android_app *app, int32_t cmd);
  void PollEvents(NATIVEwindow *windowHandle);

  static NAN_METHOD(New);

  static NAN_METHOD(CreateSwapChain);
  static NAN_METHOD(WaitGetPoses);
  static NAN_METHOD(Submit);
  static NAN_METHOD(GetRecommendedRenderTargetSize);

  OculusMobileContext();
  ~OculusMobileContext();

// protected:
  ovrMobile *ovrState;
  bool running;
  ANativeWindow *androidNativeWindow;
  ovrTextureSwapChain *swapChain;
  int swapChainLength;
  int swapChainIndex;
  ovrTracking2 tracking;
  long long frameIndex;
	double displayTime;
  int swapInterval;
};

}

#endif

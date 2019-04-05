#ifndef _OVR_SESSION_H_
#define _OVR_SESSION_H_

#include <nan.h>
#include <v8.h>
#include <webgl.h>

#include "OVR_CAPI_GL.h"

typedef struct SwapChain {
  ovrTextureSwapChain ColorTextureChain;
  ovrTextureSwapChain DepthTextureChain;
  ovrSizei textureSize;
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
  static NAN_METHOD(GetControllersInputState);
  static NAN_METHOD(GetPose);
  static NAN_METHOD(SetupSwapChain);
  static NAN_METHOD(Submit);
  static NAN_METHOD(GetRecommendedRenderTargetSize);

  /// Create a singleton reference to a constructor function.
  static inline Nan::Persistent<v8::Function>& constructor()
  {
    static Nan::Persistent<v8::Function> the_constructor;
    return the_constructor;
  }

  void DestroySession() {
    ovr_Destroy(*this->self_);
    ovr_Shutdown();
  }

  void SetupSession() {

    if (this->self_) {
      DestroySwapChain();
      DestroySession();
    }

    ovrInitParams initParams = {
      ovrInit_RequestVersion | ovrInit_MixedRendering,
      OVR_MINOR_VERSION, NULL, 0, 0
    };

    // Reinitialize Oculus runtime.
    ovr_Initialize(&initParams);

    ovrSession *session = (ovrSession *) malloc(sizeof(ovrSession));
    ovrResult result;
    ovrGraphicsLuid luid;
    result = ovr_Create(session, &luid);
    if (OVR_FAILURE(result))
    {
      Nan::ThrowError("Error creating ovr session");
      ovr_Shutdown();
      return;
    }

    this->self_ = session;
  }

  void ResetSession() {
    DestroySwapChain();
    SetupSession();
    SetupSwapChain();
  }

  void DestroySwapChain() {
    // Destroy
    ovr_DestroyTextureSwapChain(*this->self_, this->eyes[0].ColorTextureChain);
    ovr_DestroyTextureSwapChain(*this->self_, this->eyes[0].DepthTextureChain);
    ovr_DestroyTextureSwapChain(*this->self_, this->eyes[1].ColorTextureChain);
    ovr_DestroyTextureSwapChain(*this->self_, this->eyes[1].DepthTextureChain);

    glDeleteFramebuffers(1, &this->fboId);
  }

  void SetupSwapChain() {
    ovrResult result;
    ovrSizei recommenedTex0Size = ovr_GetFovTextureSize(*this->self_, ovrEye_Left, this->hmdDesc.DefaultEyeFov[ovrEye_Left], 1);
    ovrSizei recommenedTex1Size = ovr_GetFovTextureSize(*this->self_, ovrEye_Right, this->hmdDesc.DefaultEyeFov[ovrEye_Right], 1);
    ovrSizei bufferSize;

    this->eyes[0].textureSize.w = recommenedTex0Size.w;
    this->eyes[0].textureSize.h = recommenedTex0Size.h;
    this->eyes[1].textureSize.w = recommenedTex1Size.w;
    this->eyes[1].textureSize.h = recommenedTex1Size.h;

    bufferSize.w  = recommenedTex0Size.w;
    bufferSize.h = std::max(recommenedTex0Size.h, recommenedTex1Size.h);

    // Make eye render buffers
    for (int eye = 0; eye < 2; ++eye) {
      ovrTextureSwapChainDesc desc = {};
      desc.Type = ovrTexture_2D;
      desc.ArraySize = 1;
      desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
      desc.Width = bufferSize.w;
      desc.Height = bufferSize.h;
      desc.MipLevels = 1;
      desc.SampleCount = 1;
      desc.StaticImage = ovrFalse;

      result = ovr_CreateTextureSwapChainGL(*this->self_, &desc, &this->eyes[eye].ColorTextureChain);
      int length = 0;
      ovr_GetTextureSwapChainLength(*this->self_, this->eyes[eye].ColorTextureChain, &length);

      if (!OVR_SUCCESS(result)) {
        std::cout << "Error creating Oculus GL Swap Chain" << std::endl;
      } else {
        for (int i = 0; i < length; ++i) {
          GLuint textureId;
          ovr_GetTextureSwapChainBufferGL(*this->self_, this->eyes[eye].ColorTextureChain, i, &textureId);
          glBindTexture(GL_TEXTURE_2D, textureId);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
      }

      desc.Format = OVR_FORMAT_D32_FLOAT;

      result = ovr_CreateTextureSwapChainGL(*this->self_, &desc, &this->eyes[eye].DepthTextureChain);
      ovr_GetTextureSwapChainLength(*this->self_, this->eyes[eye].DepthTextureChain, &length);

      if (!OVR_SUCCESS(result)) {
      } else {
        for (int i = 0; i < length; ++i) {
          GLuint textureId;
          ovr_GetTextureSwapChainBufferGL(*this->self_, this->eyes[eye].DepthTextureChain, i, &textureId);
          glBindTexture(GL_TEXTURE_2D, textureId);

          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
      }

      glGenFramebuffers(1, &this->fboId);

    }
  }

  /// Reference to wrapped ovrSession instance.
  ovrSession * self_;
  ovrHmdDesc hmdDesc;
  EyeSwapChain eyes[2];
  ovrPosef eyeRenderPoses[2];
  GLuint fboId;
  int frameIndex;
  double sensorSampleTime;
  bool hmdMounted;
};

#endif

#include <deque>
#include <iostream>
#include <ovrsession.h>
#include <stdlib.h>
#include <mutex>
#include <node.h>
#include <thread>
#include <v8.h>

#include <Extras/OVR_Math.h>

using namespace v8;
using namespace OVR;

namespace oculusvr {
  uv_sem_t reqSem;
  uv_async_t resAsync;
  std::mutex reqMutex;
  std::mutex resMutex;
  std::deque<std::function<void()>> reqCbs;
  std::deque<std::function<void()>> resCbs;
  std::thread reqThread;

  void RunResInMainThread(uv_async_t *handle) {
    Nan::HandleScope scope;

    std::function<void()> resCb;
    {
      std::lock_guard<std::mutex> lock(resMutex);

      resCb = resCbs.front();
      resCbs.pop_front();
    }
    if (resCb) {
      resCb();
    }
  }
};

OculusVRPosRes::OculusVRPosRes(Local<Function> cb) : cb(cb) {}

OculusVRPosRes::~OculusVRPosRes() {}

NAN_MODULE_INIT(OVRSession::Init)
{
  // Create a function template that is called in JS to create this wrapper.
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);

  // Declare human-readable name for this wrapper.
  tpl->SetClassName(Nan::New("OVRSession").ToLocalChecked());

  // Declare the stored number of fields (just the wrapped C++ object).
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "CreateSwapChain", CreateSwapChain);
  Nan::SetPrototypeMethod(tpl, "ExitPresent", ExitPresent);
  Nan::SetPrototypeMethod(tpl, "GetControllersInputState", GetControllersInputState);
  Nan::SetPrototypeMethod(tpl, "GetPose", GetPose);
  Nan::SetPrototypeMethod(tpl, "Submit", Submit);
  Nan::SetPrototypeMethod(tpl, "GetRecommendedRenderTargetSize", GetRecommendedRenderTargetSize);

  // Set a static constructor function to reference the `New` function template.
  constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());

  uv_sem_init(&oculusvr::reqSem, 0);
  uv_async_init(uv_default_loop(), &oculusvr::resAsync, oculusvr::RunResInMainThread);
  oculusvr::reqThread = std::thread([]() -> void {
    for (;;) {
      uv_sem_wait(&oculusvr::reqSem);

      std::function<void()> reqCb;
      {
        std::lock_guard<std::mutex> lock(oculusvr::reqMutex);

        if (oculusvr::reqCbs.size() > 0) {
          reqCb = oculusvr::reqCbs.front();
          oculusvr::reqCbs.pop_front();
        }
      }
      if (reqCb) {
        reqCb();
      } else {
        break;
      }
    }
  });
}

//=============================================================================
Local<Object> OVRSession::NewInstance()
{
  Nan::EscapableHandleScope scope;
  Local<Function> cons = Nan::New(constructor());
  return scope.Escape(Nan::NewInstance(cons).ToLocalChecked());
}

//=============================================================================
OVRSession::OVRSession() :
  session(nullptr),
  fbo(0),
  swapChainValid(false),
  swapChainMetrics{0, 0},
  frameIndex(0),
  hmdMounted(true)
{
  ResetSession();
}

//=============================================================================
NAN_METHOD(OVRSession::New)
{
  if (!info.IsConstructCall())
  {
    Nan::ThrowError("Use the `new` keyword when creating a new instance.");
    return;
  }

  if (info.Length() != 0)
  {
    Nan::ThrowTypeError("OVRSession takes no arguments");
    return;
  }

  OVRSession *obj = new OVRSession();
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(OVRSession::GetRecommendedRenderTargetSize)
{
  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  OVRSession *s = ObjectWrap::Unwrap<OVRSession>(info.Holder());
  ovrSession &session = *s->session;
  ovrHmdDesc &hmdDesc = s->hmdDesc;

  ovrSizei leftEyeTextureSize = ovr_GetFovTextureSize(session, ovrEye_Left, hmdDesc.DefaultEyeFov[ovrEye_Left], 1);
  ovrSizei rightEyeTextureSize = ovr_GetFovTextureSize(session, ovrEye_Right, hmdDesc.DefaultEyeFov[ovrEye_Right], 1);

  int width  = leftEyeTextureSize.w;
  int height = std::max(leftEyeTextureSize.h, rightEyeTextureSize.h);

  Local<Object> result = Nan::New<Object>();
  {
    Local<String> width_prop = Nan::New<String>("width").ToLocalChecked();
    Nan::Set(result, width_prop, Nan::New<Number>(width));

    Local<String> height_prop = Nan::New<String>("height").ToLocalChecked();
    Nan::Set(result, height_prop, Nan::New<Number>(height));
  }
  info.GetReturnValue().Set(result);
}

NAN_METHOD(OVRSession::GetPose) {

  if (info.Length() != 11)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  int *frameIndex = &ObjectWrap::Unwrap<OVRSession>(info.Holder())->frameIndex;
  OVRSession *session = ObjectWrap::Unwrap<OVRSession>(info.Holder());
  Local<Float32Array> position32Array = Local<Float32Array>::Cast(info[0]);
  Local<Float32Array> orientation32Array = Local<Float32Array>::Cast(info[1]);
  Local<Float32Array> leftView32Array = Local<Float32Array>::Cast(info[2]);
  Local<Float32Array> leftProjection32Array = Local<Float32Array>::Cast(info[3]);
  Local<Float32Array> rightView32Array = Local<Float32Array>::Cast(info[4]);
  Local<Float32Array> rightProjection32Array = Local<Float32Array>::Cast(info[5]);
  // Controllers.
  Local<Float32Array> leftControllerPosition32Array = Local<Float32Array>::Cast(info[6]);
  Local<Float32Array> leftControllerOrientation32Array = Local<Float32Array>::Cast(info[7]);
  Local<Float32Array> rightControllerPosition32Array = Local<Float32Array>::Cast(info[8]);
  Local<Float32Array> rightControllerOrientation32Array = Local<Float32Array>::Cast(info[9]);

  float *positionArray = (float *)((char *)position32Array->Buffer()->GetContents().Data() + position32Array->ByteOffset());
  float *orientationArray = (float *)((char *)orientation32Array->Buffer()->GetContents().Data() + orientation32Array->ByteOffset());
  float *leftViewArray = (float *)((char *)leftView32Array->Buffer()->GetContents().Data() + leftView32Array->ByteOffset());
  float *leftProjectionArray = (float *)((char *)leftProjection32Array->Buffer()->GetContents().Data() + leftProjection32Array->ByteOffset());
  float *rightViewArray = (float *)((char *)rightView32Array->Buffer()->GetContents().Data() + rightView32Array->ByteOffset());
  float *rightProjectionArray = (float *)((char *)rightProjection32Array->Buffer()->GetContents().Data() + rightProjection32Array->ByteOffset());
  // Controllers.
  float *leftControllerPositionArray = (float *)((char *)leftControllerPosition32Array->Buffer()->GetContents().Data() + leftControllerPosition32Array->ByteOffset());
  float *leftControllerOrientationArray = (float *)((char *)leftControllerOrientation32Array->Buffer()->GetContents().Data() + leftControllerOrientation32Array->ByteOffset());
  float *rightControllerPositionArray = (float *)((char *)rightControllerPosition32Array->Buffer()->GetContents().Data() + rightControllerPosition32Array->ByteOffset());
  float *rightControllerOrientationArray = (float *)((char *)rightControllerOrientation32Array->Buffer()->GetContents().Data() + rightControllerOrientation32Array->ByteOffset());

  Local<Function> cbFn = Local<Function>::Cast(info[10]);

  OculusVRPosRes *vrPoseRes = new OculusVRPosRes(cbFn);
  {
    std::lock_guard<std::mutex> lock(oculusvr::reqMutex);
    oculusvr::reqCbs.push_back([
      session, positionArray, orientationArray, leftViewArray, leftProjectionArray,
      rightViewArray, rightProjectionArray, leftControllerPositionArray, leftControllerOrientationArray,
      rightControllerPositionArray, rightControllerOrientationArray, vrPoseRes]() -> void {

      // Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
      ovrEyeRenderDesc eyeRenderDesc[2];
      eyeRenderDesc[0] = ovr_GetRenderDesc(*session->session, ovrEye_Left, session->hmdDesc.DefaultEyeFov[0]);
      eyeRenderDesc[1] = ovr_GetRenderDesc(*session->session, ovrEye_Right, session->hmdDesc.DefaultEyeFov[1]);

      // Get eye poses, feeding in correct IPD offset
      ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose,
                                   eyeRenderDesc[1].HmdToEyePose };

      // sensorSampleTime is fed into the layer later
      ovr_GetEyePoses(
        *session->session,
        session->frameIndex,
        ovrTrue,
        HmdToEyePose,
        session->eyeRenderPoses,
        &session->sensorSampleTime
      );

      memset(positionArray, std::numeric_limits<float>::quiet_NaN(), 3);
      memset(orientationArray, std::numeric_limits<float>::quiet_NaN(), 4);
      memset(leftViewArray, std::numeric_limits<float>::quiet_NaN(), 16);
      memset(leftProjectionArray, std::numeric_limits<float>::quiet_NaN(), 16);
      memset(rightViewArray, std::numeric_limits<float>::quiet_NaN(), 16);
      memset(rightProjectionArray, std::numeric_limits<float>::quiet_NaN(), 16);

      positionArray[0] = session->eyeRenderPoses[0].Position.x;
      positionArray[1] = session->eyeRenderPoses[0].Position.y;
      positionArray[2] = session->eyeRenderPoses[0].Position.z;

      orientationArray[0] = session->eyeRenderPoses[0].Orientation.x;
      orientationArray[1] = session->eyeRenderPoses[0].Orientation.y;
      orientationArray[2] = session->eyeRenderPoses[0].Orientation.z;
      orientationArray[3] = session->eyeRenderPoses[0].Orientation.w;

      // Left view / projection
      Matrix4f rollPitchYaw = Matrix4f(session->eyeRenderPoses[0].Orientation);
      Vector3f up = Vector3f(0, 1, 0);
      Vector3f forward = Vector3f(0, 0, -1);
      Vector3f eye = session->eyeRenderPoses[0].Position;

      Matrix4f leftViewMatrix = Matrix4f(session->eyeRenderPoses[0].Orientation);
      leftViewMatrix.SetTranslation(session->eyeRenderPoses[0].Position);
      leftViewMatrix.Invert();

      Matrix4f leftProjectionMatrix = ovrMatrix4f_Projection(session->hmdDesc.DefaultEyeFov[0], 0.2f, 1000.0f, ovrProjection_None);

      rollPitchYaw = Matrix4f(session->eyeRenderPoses[1].Orientation);
      up = Vector3f(0, 1, 0);
      forward = Vector3f(0, 0, -1);
      eye = session->eyeRenderPoses[1].Position;

      Matrix4f rightViewMatrix = Matrix4f(session->eyeRenderPoses[1].Orientation);
      rightViewMatrix.SetTranslation(session->eyeRenderPoses[1].Position);
      rightViewMatrix.Invert();
      Matrix4f rightProjectionMatrix = ovrMatrix4f_Projection(session->hmdDesc.DefaultEyeFov[1], 0.2f, 1000.0f, ovrProjection_None);

      for (unsigned int v = 0; v < 4; v++) {
        for (unsigned int u = 0; u < 4; u++) {
          leftViewArray[v * 4 + u] = leftViewMatrix.M[u][v];
          leftProjectionArray[v * 4 + u] = leftProjectionMatrix.M[u][v];
          rightViewArray[v * 4 + u] = rightViewMatrix.M[u][v];
          rightProjectionArray[v * 4 + u] = rightProjectionMatrix.M[u][v];
        }
      }

      // Controllers.
      double time = ovr_GetPredictedDisplayTime(*session->session, 0);
      ovrTrackingState trackingState = ovr_GetTrackingState(*session->session, time, ovrTrue);

      ovrPoseStatef leftControllerState = trackingState.HandPoses[ovrHand_Left];
      ovrVector3f leftControllerPosition = leftControllerState.ThePose.Position;
      ovrQuatf leftControllerOrientation = leftControllerState.ThePose.Orientation;

      leftControllerPositionArray[0] = leftControllerPosition.x;
      leftControllerPositionArray[1] = leftControllerPosition.y;
      leftControllerPositionArray[2] = leftControllerPosition.z;

      leftControllerOrientationArray[0] = leftControllerOrientation.x;
      leftControllerOrientationArray[1] = leftControllerOrientation.y;
      leftControllerOrientationArray[2] = leftControllerOrientation.z;
      leftControllerOrientationArray[3] = leftControllerOrientation.w;

      ovrPoseStatef rightControllerState = trackingState.HandPoses[ovrHand_Right];
      ovrVector3f rightControllerPosition = rightControllerState.ThePose.Position;
      ovrQuatf rightControllerOrientation = rightControllerState.ThePose.Orientation;

      rightControllerPositionArray[0] = rightControllerPosition.x;
      rightControllerPositionArray[1] = rightControllerPosition.y;
      rightControllerPositionArray[2] = rightControllerPosition.z;

      rightControllerOrientationArray[0] = rightControllerOrientation.x;
      rightControllerOrientationArray[1] = rightControllerOrientation.y;
      rightControllerOrientationArray[2] = rightControllerOrientation.z;
      rightControllerOrientationArray[3] = rightControllerOrientation.w;

      {
        std::lock_guard<std::mutex> lock(oculusvr::resMutex);

        oculusvr::resCbs.push_back([vrPoseRes]() -> void {
          {
            Local<Object> asyncObject = Nan::New<Object>();
            AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "OVRSession::GetPose");

            Local<Function> cb = Nan::New(vrPoseRes->cb);
            asyncResource.MakeCallback(cb, 0, nullptr);
          }

          delete vrPoseRes;
        });
      }

      uv_async_send(&oculusvr::resAsync);

    });
  }

  uv_sem_post(&oculusvr::reqSem);
}

NAN_METHOD(OVRSession::GetControllersInputState) {
  if (info.Length() != 2)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number.");
    return;
  }

  if (!info[1]->IsFloat32Array())
  {
    Nan::ThrowTypeError("Argument[1] must be a Float32Array.");
    return;
  }

  uint32_t hand = TO_UINT32(info[0]);
  Local<Float32Array> buttons = Local<Float32Array>::Cast(info[1]);
  buttons->Set(0, Number::New(Isolate::GetCurrent(), std::numeric_limits<float>::quiet_NaN()));

  ovrSession session = *ObjectWrap::Unwrap<OVRSession>(info.Holder())->session;
  ovrInputState inputState;
  ovrControllerType controllerType = hand == 0 ? ovrControllerType_LTouch : ovrControllerType_RTouch;

  if (!(ovr_GetConnectedControllerTypes(session) & controllerType)) {
    buttons->Set(0, Number::New(Isolate::GetCurrent(), 0));
    return;
  } else {
    buttons->Set(0, Number::New(Isolate::GetCurrent(), 1));
  }

  if (OVR_SUCCESS(ovr_GetInputState(session, ovrControllerType_Touch, &inputState)))
  {
    if (hand == 0) {
      // Presses
      buttons->Set(1, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_X) ? 1 : 0));
      buttons->Set(2, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_Y) ? 1 : 0));
      buttons->Set(3, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_LThumb) ? 1 : 0));
      buttons->Set(4, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_Enter) ? 1 : 0));

      // Triggers
      buttons->Set(5, Number::New(Isolate::GetCurrent(), inputState.IndexTrigger[ovrHand_Left]));
      buttons->Set(6, Number::New(Isolate::GetCurrent(), inputState.HandTrigger[ovrHand_Left]));

      // Touches
      buttons->Set(7, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_X) ? 1 : 0));
      buttons->Set(8, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_Y) ? 1 : 0));
      buttons->Set(9, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_LThumb) ? 1 : 0));
      buttons->Set(10, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_LIndexTrigger) ? 1 : 0));

      // Thumbstick axis.
      buttons->Set(11, Number::New(Isolate::GetCurrent(), inputState.Thumbstick[ovrHand_Left].x));
      buttons->Set(12, Number::New(Isolate::GetCurrent(), inputState.Thumbstick[ovrHand_Left].y));
    } else {
      // Presses
      buttons->Set(1, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_A) ? 1 : 0));
      buttons->Set(2, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_B) ? 1 : 0));
      buttons->Set(3, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_RThumb) ? 1 : 0));
      buttons->Set(4, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_Home) ? 1 : 0));

      // Triggers
      buttons->Set(5, Number::New(Isolate::GetCurrent(), inputState.IndexTrigger[ovrHand_Right]));
      buttons->Set(6, Number::New(Isolate::GetCurrent(), inputState.HandTrigger[ovrHand_Right]));

      // Touches
      buttons->Set(7, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_A) ? 1 : 0));
      buttons->Set(8, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_B) ? 1 : 0));
      buttons->Set(9, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_RThumb) ? 1 : 0));
      buttons->Set(10, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_RIndexTrigger) ? 1 : 0));

      // Thumbstick axis.
      buttons->Set(11, Number::New(Isolate::GetCurrent(), inputState.Thumbstick[ovrHand_Right].x));
      buttons->Set(12, Number::New(Isolate::GetCurrent(), inputState.Thumbstick[ovrHand_Right].y));
    }
  }
}

NAN_METHOD(OVRSession::Submit) {

  if (info.Length() != 0) {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  OVRSession *session = ObjectWrap::Unwrap<OVRSession>(info.Holder());
  ovrSessionStatus sessionStatus;
  ovr_GetSessionStatus(*session->session, &sessionStatus);

  if (sessionStatus.HmdMounted && !session->hmdMounted) {
    session->ResetSession();
  }
  session->hmdMounted = sessionStatus.HmdMounted;

  ovr_CommitTextureSwapChain(*session->session, session->swapChain.ColorTextureChain);
  ovr_CommitTextureSwapChain(*session->session, session->swapChain.DepthTextureChain);

  ovrTimewarpProjectionDesc posTimewarpProjectionDesc = {};

  // Distortion, Present and flush/sync
  ovrLayerEyeFovDepth ld = {};
  ld.Header.Type = ovrLayerType_EyeFovDepth;
  ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
  ld.ProjectionDesc = posTimewarpProjectionDesc;

  for (int eye = 0; eye < 2; eye++) {
    ld.ColorTexture[eye] = eye == 0 ? session->swapChain.ColorTextureChain : nullptr;
    ld.DepthTexture[eye] = eye == 0 ? session->swapChain.DepthTextureChain : nullptr;
    ld.Viewport[eye] = eye == 0 ?
      OVR::Recti(0, 0, session->swapChainMetrics[0]/2, session->swapChainMetrics[1])
    :
      OVR::Recti(session->swapChainMetrics[0]/2, 0, session->swapChainMetrics[0]/2, session->swapChainMetrics[1]);
    ld.Fov[eye] = session->hmdDesc.DefaultEyeFov[eye];
    ld.RenderPose[eye] = session->eyeRenderPoses[eye];
    ld.SensorSampleTime = session->sensorSampleTime;
  }

  ovrLayerHeader *layers[] = {
    &ld.Header,
  };
  ovr_SubmitFrame(*session->session, session->frameIndex, nullptr, layers, sizeof(layers)/sizeof(layers[0]));
  session->frameIndex++;

  GLuint colorTex;
  {
    int curIndex;
    ovr_GetTextureSwapChainCurrentIndex(*session->session, session->swapChain.ColorTextureChain, &curIndex);
    ovr_GetTextureSwapChainBufferGL(*session->session, session->swapChain.ColorTextureChain, curIndex, &colorTex);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
  }
  GLuint depthStencilTex;
  {
    int curIndex;
    ovr_GetTextureSwapChainCurrentIndex(*session->session, session->swapChain.DepthTextureChain, &curIndex);
    ovr_GetTextureSwapChainBufferGL(*session->session, session->swapChain.DepthTextureChain, curIndex, &depthStencilTex);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);
  }

  Local<Array> array = Array::New(Isolate::GetCurrent(), 3);
  array->Set(0, JS_INT(session->fbo));
  array->Set(1, JS_INT(colorTex));
  array->Set(2, JS_INT(depthStencilTex));
  info.GetReturnValue().Set(array);
}

void OVRSession::DestroySession() {
  ovr_Destroy(*this->session);
  this->session = nullptr;
  ovr_Shutdown();
}

void OVRSession::ResetSession() {

  bool hadSwapChain = this->swapChainValid;
  if (hadSwapChain) {
    DestroySwapChain();
  }
  if (this->session != nullptr) {
    DestroySession();
  }

  ovrInitParams initParams = {
    ovrInit_RequestVersion | ovrInit_MixedRendering,
    OVR_MINOR_VERSION, OculusVRLogCallback, 0, 0
  };

  // Reinitialize Oculus runtime.
  ovr_Initialize(&initParams);

  ovrSession *session = (ovrSession *) malloc(sizeof(ovrSession));
  ovrGraphicsLuid luid;
  ovrResult result = ovr_Create(session, &luid);
  if (OVR_FAILURE(result))
  {
    Nan::ThrowError("Error creating ovr session");
    ovr_Shutdown();
    return;
  }

  this->session = session;
  this->hmdDesc = ovr_GetHmdDesc(*this->session);
  
  if (hadSwapChain) {
    ResetSwapChain();
  }
}

void OVRSession::ResetSwapChain() {
  if (this->swapChainValid) {
    DestroySwapChain();
  }
  
  // Framebuffer
  glGenFramebuffers(1, &this->fbo);

  // Color swap chain
  ovrTextureSwapChainDesc desc = {};
  desc.Type = ovrTexture_2D;
  desc.ArraySize = 1;
  desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
  desc.Width = this->swapChainMetrics[0];
  desc.Height = this->swapChainMetrics[1];
  desc.MipLevels = 1;
  desc.SampleCount = 1;
  desc.StaticImage = ovrFalse;

  ovrResult result = ovr_CreateTextureSwapChainGL(*this->session, &desc, &this->swapChain.ColorTextureChain);
  int length = 0;
  ovr_GetTextureSwapChainLength(*this->session, this->swapChain.ColorTextureChain, &length);

  if (!OVR_SUCCESS(result)) {
    std::cout << "Error creating Oculus GL Color Swap Chain" << std::endl;
  } else {
    for (int i = 0; i < length; ++i) {
      GLuint textureId;
      ovr_GetTextureSwapChainBufferGL(*this->session, this->swapChain.ColorTextureChain, i, &textureId);
      glBindTexture(GL_TEXTURE_2D, textureId);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
  }

  // Depth swap chain
  desc.Format = OVR_FORMAT_D24_UNORM_S8_UINT;

  result = ovr_CreateTextureSwapChainGL(*this->session, &desc, &this->swapChain.DepthTextureChain);
  ovr_GetTextureSwapChainLength(*this->session, this->swapChain.DepthTextureChain, &length);

  if (!OVR_SUCCESS(result)) {
    std::cout << "Error creating Oculus GL Depth Swap Chain" << std::endl;
  } else {
    for (int i = 0; i < length; ++i) {
      GLuint textureId;
      ovr_GetTextureSwapChainBufferGL(*this->session, this->swapChain.DepthTextureChain, i, &textureId);
      glBindTexture(GL_TEXTURE_2D, textureId);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      
    }
  }
  
  this->swapChainValid = true;
}

void OVRSession::DestroySwapChain() {
  glDeleteFramebuffers(1, &this->fbo);
  ovr_DestroyTextureSwapChain(*this->session, this->swapChain.ColorTextureChain);
  ovr_DestroyTextureSwapChain(*this->session, this->swapChain.DepthTextureChain);
  
  this->swapChainValid = false;
}

NAN_METHOD(OVRSession::CreateSwapChain) {
  if (info.Length() != 2)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[1] must be a Number.");
    return;
  }
  
  if (!info[1]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[2] must be a Number.");
    return;
  }
  
  OVRSession *session = ObjectWrap::Unwrap<OVRSession>(info.Holder());
  int width = TO_INT32(info[0]);
  int height = TO_INT32(info[1]);
  
  session->swapChainMetrics[0] = width;
  session->swapChainMetrics[1] = height;
  
  session->ResetSwapChain();

  GLuint colorTex;
  {
    int curIndex;
    ovr_GetTextureSwapChainCurrentIndex(*session->session, session->swapChain.ColorTextureChain, &curIndex);
    ovr_GetTextureSwapChainBufferGL(*session->session, session->swapChain.ColorTextureChain, curIndex, &colorTex);
    // glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
  }
  GLuint depthStencilTex;
  {
    int curIndex;
    ovr_GetTextureSwapChainCurrentIndex(*session->session, session->swapChain.DepthTextureChain, &curIndex);
    ovr_GetTextureSwapChainBufferGL(*session->session, session->swapChain.DepthTextureChain, curIndex, &depthStencilTex);
    // glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);
  }

  Local<Array> array = Array::New(Isolate::GetCurrent(), 3);
  array->Set(0, JS_INT(session->fbo));
  array->Set(1, JS_INT(colorTex));
  array->Set(2, JS_INT(depthStencilTex));
  info.GetReturnValue().Set(array);
}

NAN_METHOD(OVRSession::ExitPresent) {
  OVRSession *session = ObjectWrap::Unwrap<OVRSession>(info.Holder());

  glDeleteFramebuffers(1, &session->fbo);
  ovr_DestroyTextureSwapChain(*session->session, session->swapChain.ColorTextureChain);
  ovr_DestroyTextureSwapChain(*session->session, session->swapChain.DepthTextureChain);
}

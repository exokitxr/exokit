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
  std::thread reqThead;

  void RunResInMainThread(uv_async_t *handle) {
    Nan::HandleScope scope;

    std::function<void()> resCb;
    {
      std::lock_guard<std::mutex> lock(reqMutex);

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

  Nan::SetPrototypeMethod(tpl, "GetControllersInputState", GetControllersInputState);
  Nan::SetPrototypeMethod(tpl, "GetPose", GetPose);
  Nan::SetPrototypeMethod(tpl, "Submit", Submit);
  Nan::SetPrototypeMethod(tpl, "SetupSwapChain", SetupSwapChain);
  Nan::SetPrototypeMethod(tpl, "GetRecommendedRenderTargetSize", GetRecommendedRenderTargetSize);

  // Set a static constructor function to reference the `New` function template.
  constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());

  uv_sem_init(&oculusvr::reqSem, 0);
  uv_async_init(uv_default_loop(), &oculusvr::resAsync, oculusvr::RunResInMainThread);
  oculusvr::reqThead = std::thread([]() -> void {
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
Local<Object> OVRSession::NewInstance(ovrSession *session)
{
  Nan::EscapableHandleScope scope;
  Local<Function> cons = Nan::New(constructor());
  Local<Value> argv[1] = { Nan::New<External>(session) };
  return scope.Escape(Nan::NewInstance(cons, 1, argv).ToLocalChecked());
}

//=============================================================================
OVRSession::OVRSession(ovrSession *self)
: self_(self), frameIndex(0)
{

}

//=============================================================================
NAN_METHOD(OVRSession::New)
{
  if (!info.IsConstructCall())
  {
    Nan::ThrowError("Use the `new` keyword when creating a new instance.");
    return;
  }

  if (info.Length() != 1 || !info[0]->IsExternal())
  {
    Nan::ThrowTypeError("Argument[0] must be an `ovrSession*`.");
    return;
  }

  auto wrapped_instance = static_cast<ovrSession*>(
    Local<External>::Cast(info[0])->Value());
  OVRSession *obj = new OVRSession(wrapped_instance);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(OVRSession::SetupSwapChain)
{
  ovrResult result;
  ovrSession session = *ObjectWrap::Unwrap<OVRSession>(info.Holder())->self_;
  // Configure Stereo settings.
  ovrHmdDesc *hmdDesc = &ObjectWrap::Unwrap<OVRSession>(info.Holder())->hmdDesc;
  *hmdDesc = ovr_GetHmdDesc(session);
  GLuint *fboId = &ObjectWrap::Unwrap<OVRSession>(info.Holder())->fboId;
  EyeSwapChain *eyes = &*ObjectWrap::Unwrap<OVRSession>(info.Holder())->eyes;
  ovrSizei recommenedTex0Size = ovr_GetFovTextureSize(session, ovrEye_Left, hmdDesc->DefaultEyeFov[ovrEye_Left], 1);
  ovrSizei recommenedTex1Size = ovr_GetFovTextureSize(session, ovrEye_Right, hmdDesc->DefaultEyeFov[ovrEye_Right], 1);
  ovrSizei bufferSize;

  eyes[0].textureSize.w = recommenedTex0Size.w;
  eyes[0].textureSize.h = recommenedTex0Size.h;
  eyes[1].textureSize.w = recommenedTex1Size.w;
  eyes[1].textureSize.h = recommenedTex1Size.h;

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

    result = ovr_CreateTextureSwapChainGL(session, &desc, &eyes[eye].ColorTextureChain);
    int length = 0;
    ovr_GetTextureSwapChainLength(session, eyes[eye].ColorTextureChain, &length);

    if (!OVR_SUCCESS(result)) {
      std::cout << "Error creating Oculus GL Swap Chain" << std::endl;
    } else {
      for (int i = 0; i < length; ++i) {
        GLuint textureId;
        ovr_GetTextureSwapChainBufferGL(session, eyes[eye].ColorTextureChain, i, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
    }

    desc.Format = OVR_FORMAT_D32_FLOAT;

    result = ovr_CreateTextureSwapChainGL(session, &desc, &eyes[eye].DepthTextureChain);
    ovr_GetTextureSwapChainLength(session, eyes[eye].DepthTextureChain, &length);

    if (!OVR_SUCCESS(result)) {
    } else {
      for (int i = 0; i < length; ++i) {
        GLuint textureId;
        ovr_GetTextureSwapChainBufferGL(session, eyes[eye].DepthTextureChain, i, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
    }

    glGenFramebuffers(1, fboId);

  }
}

NAN_METHOD(OVRSession::GetRecommendedRenderTargetSize)
{
  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  ovrSession session = *ObjectWrap::Unwrap<OVRSession>(info.Holder())->self_;
  ovrHmdDesc hmdDesc = ObjectWrap::Unwrap<OVRSession>(info.Holder())->hmdDesc;

  // Oculus: Initialize Swap Chain
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
  ovrSession session = *ObjectWrap::Unwrap<OVRSession>(info.Holder())->self_;
  EyeSwapChain *eyes = &*ObjectWrap::Unwrap<OVRSession>(info.Holder())->eyes;
  ovrPosef *eyeRenderPoses = &*ObjectWrap::Unwrap<OVRSession>(info.Holder())->eyeRenderPoses;
  ovrHmdDesc hmdDesc = ObjectWrap::Unwrap<OVRSession>(info.Holder())->hmdDesc;
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
      session, frameIndex, eyes, eyeRenderPoses, hmdDesc, positionArray, orientationArray, leftViewArray, leftProjectionArray,
      rightViewArray, rightProjectionArray, leftControllerPositionArray, leftControllerOrientationArray,
      rightControllerPositionArray, rightControllerOrientationArray, vrPoseRes]() -> void {

      // Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
      ovrEyeRenderDesc eyeRenderDesc[2];
      eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
      eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

      // Get eye poses, feeding in correct IPD offset
      ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose,
                                   eyeRenderDesc[1].HmdToEyePose };

      double sensorSampleTime;    // sensorSampleTime is fed into the layer later
      ovr_GetEyePoses(session, *frameIndex, ovrTrue, HmdToEyePose, eyeRenderPoses, &sensorSampleTime);

      memset(positionArray, std::numeric_limits<float>::quiet_NaN(), 3);
      memset(orientationArray, std::numeric_limits<float>::quiet_NaN(), 4);
      memset(leftViewArray, std::numeric_limits<float>::quiet_NaN(), 16);
      memset(leftProjectionArray, std::numeric_limits<float>::quiet_NaN(), 16);
      memset(rightViewArray, std::numeric_limits<float>::quiet_NaN(), 16);
      memset(rightProjectionArray, std::numeric_limits<float>::quiet_NaN(), 16);

      positionArray[0] = eyeRenderPoses[0].Position.x;
      positionArray[1] = eyeRenderPoses[0].Position.y;
      positionArray[2] = eyeRenderPoses[0].Position.z;

      orientationArray[0] = eyeRenderPoses[0].Orientation.x;
      orientationArray[1] = eyeRenderPoses[0].Orientation.y;
      orientationArray[2] = eyeRenderPoses[0].Orientation.z;
      orientationArray[3] = eyeRenderPoses[0].Orientation.w;

      // Left view / projection
      Matrix4f rollPitchYaw = Matrix4f(eyeRenderPoses[0].Orientation);
      Vector3f up = Vector3f(0, 1, 0);
      Vector3f forward = Vector3f(0, 0, -1);
      Vector3f eye = eyeRenderPoses[0].Position;

      Matrix4f leftViewMatrix = Matrix4f(eyeRenderPoses[0].Orientation);
      leftViewMatrix.SetTranslation(eyeRenderPoses[0].Position);
      leftViewMatrix.Invert();

      Matrix4f leftProjectionMatrix = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[0], 0.2f, 1000.0f, ovrProjection_None);

      rollPitchYaw = Matrix4f(eyeRenderPoses[1].Orientation);
      up = Vector3f(0, 1, 0);
      forward = Vector3f(0, 0, -1);
      eye = eyeRenderPoses[1].Position;

      Matrix4f rightViewMatrix = Matrix4f(eyeRenderPoses[1].Orientation);
      rightViewMatrix.SetTranslation(eyeRenderPoses[1].Position);
      rightViewMatrix.Invert();
      Matrix4f rightProjectionMatrix = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[1], 0.2f, 1000.0f, ovrProjection_None);

      for (unsigned int v = 0; v < 4; v++) {
        for (unsigned int u = 0; u < 4; u++) {
          leftViewArray[v * 4 + u] = leftViewMatrix.M[u][v];
          leftProjectionArray[v * 4 + u] = leftProjectionMatrix.M[u][v];
          rightViewArray[v * 4 + u] = rightViewMatrix.M[u][v];
          rightProjectionArray[v * 4 + u] = rightProjectionMatrix.M[u][v];
        }
      }

      // Controllers.
      double time = ovr_GetPredictedDisplayTime(session, 0);
      ovrTrackingState trackingState = ovr_GetTrackingState(session, time, ovrTrue);

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
}

NAN_METHOD(OVRSession::GetControllersInputState) {
  if (info.Length() != 2)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  uint32_t hand = info[0]->Uint32Value();
  Local<Float32Array> buttons = Local<Float32Array>::Cast(info[1]);
  buttons->Set(0, Number::New(Isolate::GetCurrent(), std::numeric_limits<float>::quiet_NaN()));

  ovrSession session = *ObjectWrap::Unwrap<OVRSession>(info.Holder())->self_;
  ovrInputState inputState;

  if (OVR_SUCCESS(ovr_GetInputState(session, ovrControllerType_Touch, &inputState)))
  {
    if (hand == 0) {
      // Presses
      buttons->Set(0, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_X) ? 1 : 0));
      buttons->Set(1, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_Y) ? 1 : 0));
      buttons->Set(2, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_LThumb) ? 1 : 0));
      buttons->Set(3, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_Enter) ? 1 : 0));

      // Triggers
      buttons->Set(4, Number::New(Isolate::GetCurrent(), inputState.IndexTrigger[ovrHand_Left]));
      buttons->Set(5, Number::New(Isolate::GetCurrent(), inputState.HandTrigger[ovrHand_Left]));

      // Touches
      buttons->Set(6, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_X) ? 1 : 0));
      buttons->Set(7, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_Y) ? 1 : 0));
      buttons->Set(8, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_LThumb) ? 1 : 0));
      buttons->Set(9, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_LIndexTrigger) ? 1 : 0));

      // Thumbstick axis.
      buttons->Set(10, Number::New(Isolate::GetCurrent(), inputState.Thumbstick[ovrHand_Left].x));
      buttons->Set(11, Number::New(Isolate::GetCurrent(), inputState.Thumbstick[ovrHand_Left].y));
    } else {
      // Presses
      buttons->Set(0, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_A) ? 1 : 0));
      buttons->Set(1, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_B) ? 1 : 0));
      buttons->Set(2, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_RThumb) ? 1 : 0));
      buttons->Set(3, Number::New(Isolate::GetCurrent(), (inputState.Buttons & ovrButton_Home) ? 1 : 0));

      // Triggers
      buttons->Set(4, Number::New(Isolate::GetCurrent(), inputState.IndexTrigger[ovrHand_Right]));
      buttons->Set(5, Number::New(Isolate::GetCurrent(), inputState.HandTrigger[ovrHand_Right]));

      // Touches
      buttons->Set(6, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_A) ? 1 : 0));
      buttons->Set(7, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_B) ? 1 : 0));
      buttons->Set(8, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_RThumb) ? 1 : 0));
      buttons->Set(9, Number::New(Isolate::GetCurrent(), (inputState.Touches & ovrTouch_RIndexTrigger) ? 1 : 0));

      // Thumbstick axis.
      buttons->Set(10, Number::New(Isolate::GetCurrent(), inputState.Thumbstick[ovrHand_Right].x));
      buttons->Set(11, Number::New(Isolate::GetCurrent(), inputState.Thumbstick[ovrHand_Right].y));
    }
  }
}

NAN_METHOD(OVRSession::Submit)
{

  if (info.Length() != 5)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!(info[0]->IsObject() && info[1]->IsNumber()))
  {
    Nan::ThrowError("Expected arguments (object, number).");
    return;
  }

  WebGLRenderingContext *gl = node::ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  GLuint textureSource = info[1]->Uint32Value();
  GLuint fboIdSource = info[2]->Uint32Value();
  int width = info[3]->Uint32Value();
  int height = info[4]->Uint32Value();

  ovrPosef *eyeRenderPoses = &*ObjectWrap::Unwrap<OVRSession>(info.Holder())->eyeRenderPoses;
  int *frameIndex = &ObjectWrap::Unwrap<OVRSession>(info.Holder())->frameIndex;
  GLuint fboId = ObjectWrap::Unwrap<OVRSession>(info.Holder())->fboId;
  ovrSession session = *ObjectWrap::Unwrap<OVRSession>(info.Holder())->self_;
  double sensorSampleTime = ObjectWrap::Unwrap<OVRSession>(info.Holder())->sensorSampleTime;
  EyeSwapChain *eyes = ObjectWrap::Unwrap<OVRSession>(info.Holder())->eyes;
  ovrHmdDesc hmdDesc = ObjectWrap::Unwrap<OVRSession>(info.Holder())->hmdDesc;

  // Eye textures clear.
  GLuint colorTextureId;
  GLuint depthTextureId;
  ovrTimewarpProjectionDesc posTimewarpProjectionDesc = {};

  for (int eye = 0; eye < 2; ++eye) {

    {
      int curIndex;
      ovr_GetTextureSwapChainCurrentIndex(session, eyes[eye].ColorTextureChain, &curIndex);
      ovr_GetTextureSwapChainBufferGL(session, eyes[eye].ColorTextureChain, curIndex, &colorTextureId);
    }
    {
      int curIndex;
      ovr_GetTextureSwapChainCurrentIndex(session, eyes[eye].DepthTextureChain, &curIndex);
      ovr_GetTextureSwapChainBufferGL(session, eyes[eye].DepthTextureChain, curIndex, &depthTextureId);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboIdSource);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextureId, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId, 0);

    glBlitFramebuffer(
      eye == 0 ? 0 : width/2, 0,
      eye == 0 ? width/2 : width, height,
      0, 0,
      eyes[eye].textureSize.w, eyes[eye].textureSize.h,
      GL_COLOR_BUFFER_BIT,
      GL_LINEAR
    );

    ovr_CommitTextureSwapChain(session, eyes[eye].ColorTextureChain);
    ovr_CommitTextureSwapChain(session, eyes[eye].DepthTextureChain);

    if (gl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
    }

    if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
    }

  }

  // Distortion, Present and flush/sync
  ovrLayerEyeFovDepth ld = {};
  ld.Header.Type  = ovrLayerType_EyeFovDepth;
  ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
  ld.ProjectionDesc = posTimewarpProjectionDesc;

  for (int eye = 0; eye < 2; ++eye)
  {
    ld.ColorTexture[eye] = eyes[eye].ColorTextureChain;
    ld.DepthTexture[eye] = eyes[eye].DepthTextureChain;
    ld.Viewport[eye]     = OVR::Recti(eyes[eye].textureSize);
    ld.Fov[eye]          = hmdDesc.DefaultEyeFov[eye];
    ld.RenderPose[eye]   = eyeRenderPoses[eye];
    ld.SensorSampleTime  = sensorSampleTime;
  }

  ovrLayerHeader* layers = &ld.Header;
  ovrResult result = ovr_SubmitFrame(session, *frameIndex, nullptr, &layers, 1);

  *frameIndex += 1;
}
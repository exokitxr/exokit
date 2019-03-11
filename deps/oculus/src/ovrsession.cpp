#include <stdlib.h>
#include <iostream>
#include <ovrsession.h>
#include <node.h>
#include <v8.h>

#include <Extras/OVR_Math.h>

using namespace v8;
using namespace OVR;

NAN_MODULE_INIT(OVRSession::Init)
{
  // Create a function template that is called in JS to create this wrapper.
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);

  // Declare human-readable name for this wrapper.
  tpl->SetClassName(Nan::New("OVRSession").ToLocalChecked());

  // Declare the stored number of fields (just the wrapped C++ object).
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "GetPose", GetPose);
  Nan::SetPrototypeMethod(tpl, "Submit", Submit);
  Nan::SetPrototypeMethod(tpl, "SetupSwapChain", SetupSwapChain);
  Nan::SetPrototypeMethod(tpl, "RequestGetPoses", RequestGetPoses);
  Nan::SetPrototypeMethod(tpl, "GetRecommendedRenderTargetSize", GetRecommendedRenderTargetSize);

  // Set a static constructor function to reference the `New` function template.
  constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
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

  bufferSize.w  = recommenedTex0Size.w + recommenedTex1Size.w;
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
      std::cout << "SWAP CHAIN ERROR " << result << std::endl;
    } else {
      std::cout << "SWAP CHAIN CREATED SUCCESS" << std::endl;
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
      std::cout << "SWAP CHAIN ERROR " << result << std::endl;
    } else {
      std::cout << "SWAP CHAIN CREATED SUCCESS" << std::endl;
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

  int width  = leftEyeTextureSize.w + rightEyeTextureSize.w;
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

NAN_METHOD(OVRSession::RequestGetPoses) {
  if (info.Length() != 3)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  int *frameIndex = &ObjectWrap::Unwrap<OVRSession>(info.Holder())->frameIndex;
  ovrSession session = *ObjectWrap::Unwrap<OVRSession>(info.Holder())->self_;
  EyeSwapChain *eyes = &*ObjectWrap::Unwrap<OVRSession>(info.Holder())->eyes;
  ovrPosef *eyeRenderPoses = &*ObjectWrap::Unwrap<OVRSession>(info.Holder())->eyeRenderPoses;
  ovrHmdDesc hmdDesc = ObjectWrap::Unwrap<OVRSession>(info.Holder())->hmdDesc;
  Local<Float32Array> hmdFloat32Array = Local<Float32Array>::Cast(info[0]);
  Local<Float32Array> leftControllerFloat32Array = Local<Float32Array>::Cast(info[1]);
  Local<Float32Array> rightControllerFloat32Array = Local<Float32Array>::Cast(info[2]);
  Local<Function> cbFn = Local<Function>::Cast(info[3]);

  float *hmdArray = (float *)((char *)hmdFloat32Array->Buffer()->GetContents().Data() + hmdFloat32Array->ByteOffset());
  float *leftControllerArray = (float *)((char *)leftControllerFloat32Array->Buffer()->GetContents().Data() + leftControllerFloat32Array->ByteOffset());
  float *rightControllerArray = (float *)((char *)rightControllerFloat32Array->Buffer()->GetContents().Data() + rightControllerFloat32Array->ByteOffset());

  // Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
  ovrEyeRenderDesc eyeRenderDesc[2];
  eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
  eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

  // Get eye poses, feeding in correct IPD offset
  ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose,
                               eyeRenderDesc[1].HmdToEyePose };

  double sensorSampleTime;    // sensorSampleTime is fed into the layer later
  ovr_GetEyePoses(session, *frameIndex, ovrTrue, HmdToEyePose, eyeRenderPoses, &sensorSampleTime);

  memset(hmdArray, std::numeric_limits<float>::quiet_NaN(), 16);
  memset(leftControllerArray, std::numeric_limits<float>::quiet_NaN(), 16);
  memset(rightControllerArray, std::numeric_limits<float>::quiet_NaN(), 16);

  Matrix4f poseMatrix = Matrix4f(eyeRenderPoses[0].Orientation);
  poseMatrix.SetTranslation(eyeRenderPoses[0].Position);

  for (unsigned int v = 0; v < 4; v++) {
    for (unsigned int u = 0; u < 4; u++) {
      hmdArray[v * 4 + u] = poseMatrix.M[u][v];
    }
  }
}

NAN_METHOD(OVRSession::GetPose) {
  if (info.Length() != 6)
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

  float *positionArray = (float *)((char *)position32Array->Buffer()->GetContents().Data() + position32Array->ByteOffset());
  float *orientationArray = (float *)((char *)orientation32Array->Buffer()->GetContents().Data() + orientation32Array->ByteOffset());
  float *leftViewArray = (float *)((char *)leftView32Array->Buffer()->GetContents().Data() + leftView32Array->ByteOffset());
  float *leftProjectionArray = (float *)((char *)leftProjection32Array->Buffer()->GetContents().Data() + leftProjection32Array->ByteOffset());
  float *rightViewArray = (float *)((char *)rightView32Array->Buffer()->GetContents().Data() + rightView32Array->ByteOffset());
  float *rightProjectionArray = (float *)((char *)rightProjection32Array->Buffer()->GetContents().Data() + rightProjection32Array->ByteOffset());


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

  // std::cout << "--- Matrix View ---" << std::endl;
  // std::cout << rightViewMatrix.M[0][0] << ' ' << rightViewMatrix.M[0][1] << ' ' << rightViewMatrix.M[0][2] << ' ' << rightViewMatrix.M[0][3] << ' ' << std::endl;
  // std::cout << rightViewMatrix.M[1][0] << ' ' << rightViewMatrix.M[1][1] << ' ' << rightViewMatrix.M[1][2] << ' ' << rightViewMatrix.M[1][3] << ' ' << std::endl;
  // std::cout << rightViewMatrix.M[2][0] << ' ' << rightViewMatrix.M[2][1] << ' ' << rightViewMatrix.M[2][2] << ' ' << rightViewMatrix.M[2][3] << ' ' << std::endl;
  // std::cout << rightViewMatrix.M[3][0] << ' ' << rightViewMatrix.M[3][1] << ' ' << rightViewMatrix.M[3][2] << ' ' << rightViewMatrix.M[3][3] << ' ' << std::endl;
  // std::cout << "--- End Matrix ---" << std::endl;

  // std::cout << "--- Matrix Projection ---" << std::endl;
  // std::cout << rightProjectionMatrix.M[0][0] << ' ' << rightProjectionMatrix.M[0][1] << ' ' << rightProjectionMatrix.M[0][2] << ' ' << rightProjectionMatrix.M[0][3] << ' ' << std::endl;
  // std::cout << rightProjectionMatrix.M[1][0] << ' ' << rightProjectionMatrix.M[1][1] << ' ' << rightProjectionMatrix.M[1][2] << ' ' << rightProjectionMatrix.M[1][3] << ' ' << std::endl;
  // std::cout << rightProjectionMatrix.M[2][0] << ' ' << rightProjectionMatrix.M[2][1] << ' ' << rightProjectionMatrix.M[2][2] << ' ' << rightProjectionMatrix.M[2][3] << ' ' << std::endl;
  // std::cout << rightProjectionMatrix.M[3][0] << ' ' << rightProjectionMatrix.M[3][1] << ' ' << rightProjectionMatrix.M[3][2] << ' ' << rightProjectionMatrix.M[3][3] << ' ' << std::endl;
  // std::cout << "--- End Matrix ---" << std::endl;

  // std::cout << "--- FOV ---" << std::endl;
  // std::cout << hmdDesc.DefaultEyeFov[1].LeftTan << std::endl;
  // std::cout << hmdDesc.DefaultEyeFov[1].RightTan << std::endl;
  // std::cout << hmdDesc.DefaultEyeFov[1].UpTan << std::endl;
  // std::cout << hmdDesc.DefaultEyeFov[1].DownTan << std::endl;

  for (unsigned int v = 0; v < 4; v++) {
    for (unsigned int u = 0; u < 4; u++) {
      leftViewArray[v * 4 + u] = leftViewMatrix.M[u][v];
      leftProjectionArray[v * 4 + u] = leftProjectionMatrix.M[u][v];
      rightViewArray[v * 4 + u] = rightViewMatrix.M[u][v];
      rightProjectionArray[v * 4 + u] = rightProjectionMatrix.M[u][v];
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
  int canvasWidth = info[3]->Uint32Value();
  int canvasHeight = info[4]->Uint32Value();

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
  glClearColor(1.0, 0.0, 1.0, 1.0);

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

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextureId, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId, 0);

    // glBlitNamedFramebuffer(
    //   fboIdSource,
    //   fboId,
    //   eye == 0 ? 0 : canvasWidth / 2, 0,
    //   eye == 0 ? canvasWidth / 2 : canvasWidth, canvasHeight,
    //   0, 0, eyes[eye].textureSize.w, eyes[eye].textureSize.h,
    //   GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
    //   GL_LINEAR);

    glViewport(0, 0, eyes[eye].textureSize.w, eyes[eye].textureSize.h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_FRAMEBUFFER_SRGB);

    ovr_CommitTextureSwapChain(session, eyes[eye].ColorTextureChain);
    ovr_CommitTextureSwapChain(session, eyes[eye].DepthTextureChain);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

    if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D)) {
      glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D));
    } else {
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE)) {
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE));
    } else {
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    }
    if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP)) {
      glBindTexture(GL_TEXTURE_CUBE_MAP, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP));
    } else {
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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
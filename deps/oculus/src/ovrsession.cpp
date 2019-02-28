#include <stdlib.h>
#include <ovrsession.h>
#include <node.h>
#include <v8.h>

using namespace v8;

NAN_MODULE_INIT(OVRSession::Init)
{
  // Create a function template that is called in JS to create this wrapper.
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);

  // Declare human-readable name for this wrapper.
  tpl->SetClassName(Nan::New("OVRSession").ToLocalChecked());

  // Declare the stored number of fields (just the wrapped C++ object).
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

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
: self_(self)
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
  auto session = static_cast<ovrSession*>(
    Local<External>::Cast(info[0])->Value());

  // Configure Stereo settings.
  ovrHmdDesc hmdDesc = ovr_GetHmdDesc(*session);
  ovrSizei recommenedTex0Size = ovr_GetFovTextureSize(*session, ovrEye_Left, hmdDesc.DefaultEyeFov[ovrEye_Left], 1);
  ovrSizei recommenedTex1Size = ovr_GetFovTextureSize(*session, ovrEye_Right, hmdDesc.DefaultEyeFov[ovrEye_Right], 1);
  ovrSizei bufferSize;
  bufferSize.w  = recommenedTex0Size.w + recommenedTex1Size.w;
  bufferSize.h = std::max(recommenedTex0Size.h, recommenedTex1Size.h);

  ovrTextureSwapChain textureSwapChain;

  ovrTextureSwapChainDesc desc = {};
  desc.Type = ovrTexture_2D;
  desc.ArraySize = 1;
  desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
  desc.Width = bufferSize.w;
  desc.Height = bufferSize.h;
  desc.MipLevels = 1;
  desc.SampleCount = 1;
  desc.StaticImage = ovrFalse;

  if (ovr_CreateTextureSwapChainGL(*session, &desc, &textureSwapChain) == ovrSuccess)
  {
      unsigned int texId;
      ovr_GetTextureSwapChainBufferGL(*session, textureSwapChain, 0, &texId);
      info.GetReturnValue().Set(texId);
  }
}
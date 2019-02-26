#include <oculus-bindings.h>

#include <nan.h>

#include <node.h>

// Include the Oculus SDK
#include "OVR_CAPI_GL.h"
#include <v8.h>

NAN_METHOD(Oculus_Init)
{
  bool returnValue;

  ovrResult result = ovr_Initialize(nullptr);
  if (OVR_FAILURE(result)) { return; }

  ovrSession session;
  ovrGraphicsLuid luid;
  result = ovr_Create(&session, &luid);
  if (OVR_FAILURE(result))
  {
    ovr_Shutdown();
    return;
  }

  ovrHmdDesc desc = ovr_GetHmdDesc(session);
  if (desc.Type == ovrHmd_None) {
    returnValue = false;
  } else {
    returnValue = true;
  }

  ovr_Destroy(session);
  ovr_Shutdown();

  info.GetReturnValue().Set(Nan::New<Boolean>(returnValue));
}

Local<Object> makeOculusVr() {
  v8::EscapableHandleScope scope(Isolate::GetCurrent());

  Local<Object> exports = Object::New(Isolate::GetCurrent());

  exports->Set(Nan::New("Oculus_Init").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(Oculus_Init)->GetFunction());

  return scope.Escape(exports);
}
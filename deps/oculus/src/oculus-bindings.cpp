#include <oculus-bindings.h>

#include <nan.h>

#include <node.h>

// Include the Oculus SDK
#include "ovrsession.h"
#include <v8.h>

using namespace v8;

NAN_METHOD(Oculus_Init)
{
  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  ovrSession * session = (ovrSession *) malloc(sizeof(ovrSession));
  ovrResult result = ovr_Initialize(nullptr);
  if (OVR_FAILURE(result)) { return; }

  ovrGraphicsLuid luid;
  result = ovr_Create(session, &luid);
  if (OVR_FAILURE(result))
  {
    ovr_Shutdown();
    return;
  }

  auto sessionResult = OVRSession::NewInstance(session);
  info.GetReturnValue().Set(sessionResult);
}

NAN_METHOD(Oculus_IsHmdPresent)
{
  bool returnValue;

  ovrSession session;
  ovrResult result = ovr_Initialize(nullptr);
  if (OVR_FAILURE(result)) { return; }

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
  exports->Set(Nan::New("Oculus_IsHmdPresent").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(Oculus_IsHmdPresent)->GetFunction());

  return scope.Escape(exports);
}
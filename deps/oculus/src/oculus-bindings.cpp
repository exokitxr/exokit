#include <oculus-bindings.h>

#include <nan.h>
#include <iostream>

#include <node.h>

// Include the Oculus SDK
#include "ovrsession.h"
#include <v8.h>

using namespace v8;

bool returnValue;

NAN_METHOD(Oculus_Init)
{
  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

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

  v8::Local<v8::Object> s = v8::Object::New(v8::Isolate::GetCurrent());
  OVRSession::Init(s);

  auto sessionResult = OVRSession::NewInstance(session);
  info.GetReturnValue().Set(sessionResult);
}

NAN_METHOD(Oculus_IsHmdPresent)
{
  bool returnValue;

  ovrResult result = ovr_Initialize(nullptr);
  if (OVR_FAILURE(result)) {
    if (result != ovrError_LibLoad) {
      std::cout << "LibOVR didn't initialize" << std::endl;
    }
    return;
  }

  ovrHmdDesc desc = ovr_GetHmdDesc(nullptr);
  if (desc.Type == ovrHmd_None) {
    returnValue = false;
  } else {
    returnValue = true;
  }

  info.GetReturnValue().Set(Nan::New<Boolean>(returnValue));
}

Local<Object> makeOculusVR() {
  v8::EscapableHandleScope scope(Isolate::GetCurrent());

  Local<Object> exports = Object::New(Isolate::GetCurrent());

  exports->Set(Nan::New("Oculus_Init").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(Oculus_Init)->GetFunction());
  exports->Set(Nan::New("Oculus_IsHmdPresent").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(Oculus_IsHmdPresent)->GetFunction());

  return scope.Escape(exports);
}
#include <oculus-bindings.h>

#include <nan.h>
#include <iostream>

#include <node.h>

// Include the Oculus SDK
#include "ovrsession.h"
#include <v8.h>

using namespace v8;

bool oculusInitialized = false;

NAN_METHOD(Oculus_Init)
{
  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  oculusInitialized = true;

  v8::Local<v8::Object> s = v8::Object::New(v8::Isolate::GetCurrent());
  OVRSession::Init(s);

  auto sessionResult = OVRSession::NewInstance();
  info.GetReturnValue().Set(sessionResult);
}

NAN_METHOD(Oculus_IsHmdPresent)
{
  bool returnValue = false;

  // If Oculus not initalized.
  // Initialized briefly to query connected headsets.
  if (oculusInitialized == false) {
    // Prevents Oculus app initialization
    // Just query if headset present.
    ovrInitParams initParams = {
      ovrInit_RequestVersion | ovrInit_Invisible,
      OVR_MINOR_VERSION, NULL, 0, 0
    };

    ovr_Initialize(&initParams);
  }

  ovrHmdDesc desc = ovr_GetHmdDesc(nullptr);
  if (desc.Type != ovrHmd_None) {
    returnValue = true;
  }

  // Shutdown if
  if (oculusInitialized == false) {
    ovr_Shutdown();
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
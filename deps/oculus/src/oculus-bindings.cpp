#include <oculus-bindings.h>

#include <nan.h>
#include <iostream>

#include <node.h>

// Include the Oculus SDK
#include "ovrsession.h"
#include <v8.h>

using namespace v8;

namespace oculusvr {

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
  const ovrDetectResult &detectResult = ovr_Detect(0);
  info.GetReturnValue().Set(Nan::New<Boolean>(detectResult.IsOculusHMDConnected));
}

};

Local<Object> makeOculusVR() {
  v8::EscapableHandleScope scope(Isolate::GetCurrent());

  Local<Object> exports = Object::New(Isolate::GetCurrent());
  Nan::SetMethod(exports, "Oculus_Init", oculusvr::Oculus_Init);
  Nan::SetMethod(exports, "Oculus_IsHmdPresent", oculusvr::Oculus_IsHmdPresent);
  return scope.Escape(exports);
}
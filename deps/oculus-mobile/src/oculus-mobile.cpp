#include <oculus-mobile.h>

#include <nan.h>
#include <iostream>

#include <node.h>
#include <v8.h>

#include "VrApi.h"
#include "VrApi_Helpers.h"
#include "VrApi_SystemUtils.h"
#include "VrApi_Input.h"

using namespace v8;

NAN_METHOD(OculusMobile_Init) {
  std::cout << "OculusMobile_Init" << std::endl;
  /* if (info.Length() != 0)
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
  info.GetReturnValue().Set(sessionResult); */
}

NAN_METHOD(OculusMobile_Shutdown) {
  std::cout << "OculusMobile_Shutdown" << std::endl;
  /* if (info.Length() != 0)
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
  info.GetReturnValue().Set(sessionResult); */
}

NAN_METHOD(OculusMobile_IsHmdPresent) {
  std::cout << "OculusMobile_IsHmdPresent" << std::endl;
  info.GetReturnValue().Set(Nan::New<Boolean>(true));
  /* bool returnValue = false;

  ovr_Initialize(nullptr);

  ovrHmdDesc desc = ovr_GetHmdDesc(nullptr);
  if (desc.Type != ovrHmd_None) {
    returnValue = true;
  }

  info.GetReturnValue().Set(Nan::New<Boolean>(returnValue)); */
}

Local<Object> makeOculusMobileVr() {
  v8::EscapableHandleScope scope(Isolate::GetCurrent());

  Local<Object> exports = Object::New(Isolate::GetCurrent());

  exports->Set(Nan::New("OculusMobile_Init").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(OculusMobile_Init)->GetFunction());
  exports->Set(Nan::New("OculusMobile_Shutdown").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(OculusMobile_Shutdown)->GetFunction());
  exports->Set(Nan::New("OculusMobile_IsHmdPresent").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(OculusMobile_IsHmdPresent)->GetFunction());

  return scope.Escape(exports);
}

#ifndef _OCULUS_MOBILE_H_
#define _OCULUS_MOBILE_H_

#include <v8.h>
#include <nan.h>

using namespace v8;

/// inline IVRSystem *OculusMobile_Init( EVRInitError *peError, EVRApplicationType eApplicationType );
NAN_METHOD(OculusMobile_Init);
NAN_METHOD(OculusMobile_IsHmdPresent);

Local<Object> makeOculusMobileVr();

#endif

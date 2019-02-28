#ifndef _OCULUS_BINDINGS_H_
#define _OCULUS_BINDINGS_H_

#include <v8.h>
#include <nan.h>

using namespace v8;

/// inline IVRSystem *Oculus_Init( EVRInitError *peError, EVRApplicationType eApplicationType );
NAN_METHOD(Oculus_Init);
NAN_METHOD(Oculus_IsHmdPresent);

Local<Object> makeOculusVr();

#endif

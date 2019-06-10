#ifndef _OCULUS_BINDINGS_H_
#define _OCULUS_BINDINGS_H_

#include <v8.h>
#include <nan.h>

using namespace v8;

namespace oculusvr {

NAN_METHOD(Oculus_Init);
NAN_METHOD(Oculus_IsHmdPresent);

};

Local<Object> makeOculusVR();

#endif

#ifndef _OCULUS_MOBILE_H_
#define _OCULUS_MOBILE_H_

#include <v8.h>
#include <nan.h>

#include <defines.h>

#include "VrApi.h"
#include "VrApi_Helpers.h"
#include "VrApi_SystemUtils.h"
#include "VrApi_Input.h"

using namespace v8;

extern struct android_app *androidApp;

namespace oculusmobile {

extern ovrJava java;
extern Nan::Persistent<v8::Function> oculusMobileContextConstructor;

/// inline IVRSystem *OculusMobile_Init( EVRInitError *peError, EVRApplicationType eApplicationType );
NAN_METHOD(OculusMobile_Init);
NAN_METHOD(OculusMobile_IsHmdPresent);

}

Local<Object> makeOculusMobileVr();

#endif

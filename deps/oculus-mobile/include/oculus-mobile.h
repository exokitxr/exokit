#ifndef _OCULUS_MOBILE_H_
#define _OCULUS_MOBILE_H_

#include <unistd.h>
#include <android/native_window_jni.h>
#include <android_native_app_glue.h>

#include <v8.h>
#include <nan.h>

#include <defines.h>

#include <exout>

#include "VrApi.h"
#include "VrApi_Types.h"
#include "VrApi_Helpers.h"
#include "VrApi_SystemUtils.h"
#include "VrApi_Input.h"

using namespace v8;

extern struct android_app *androidApp;
extern JNIEnv *androidJniEnv;

namespace oculusmobile {

extern ovrJava java;

extern Nan::Persistent<v8::Function> oculusMobileContextConstructor;
extern Nan::Persistent<v8::Object> oculusMobileContext;

/// inline IVRSystem *OculusMobile_Init( EVRInitError *peError, EVRApplicationType eApplicationType );
NAN_METHOD(OculusMobile_Init);
NAN_METHOD(OculusMobile_IsHmdPresent);
NAN_METHOD(OculusMobile_GetDeviceType);

}

Local<Object> makeOculusMobileVr();

#endif

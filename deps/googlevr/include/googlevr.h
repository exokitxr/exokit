#ifndef _GOOGLEVR_H_
#define _GOOGLEVR_H_

#include <v8.h>
#include <nan.h>

#include <defines.h>

#include <android/native_window_jni.h>
#include <android_native_app_glue.h>
#include <exout>

#include "gvr.h"
#include "gvr_audio.h"
#include "gvr_audio_surround.h"
#include "gvr_beta.h"
#include "gvr_controller.h"
#include "gvr_gesture.h"
#include "gvr_types.h"
#include "gvr_version.h"

using namespace v8;

extern struct android_app *androidApp;
extern JNIEnv *androidJniEnv;

namespace googlevr {

extern Nan::Persistent<v8::Function> googleVrContextConstructor;
extern Nan::Persistent<v8::Object> googleVrContext;

NAN_METHOD(GoogleVr_Init);
NAN_METHOD(GoogleVr_IsHmdPresent);

}

Local<Object> makeGoogleVr();

#endif

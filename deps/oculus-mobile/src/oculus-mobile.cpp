#ifdef ANDROID

#include <oculus-mobile.h>
#include <oculus-context.h>
#include <android_native_app_glue.h>

#include <nan.h>
#include <exout>

#include <node.h>
#include <v8.h>

namespace oculusmobile {

ovrJava java;
Nan::Persistent<v8::Function> oculusMobileContextConstructor;

NAN_METHOD(OculusMobile_Init) {
  std::cout << "OculusMobile_Init 1" << std::endl;

  java.Vm = androidApp->activity->vm;
  java.ActivityObject = androidApp->activity->clazz;

  const ovrInitParms initParms = vrapi_DefaultInitParms(&java);
	int32_t initResult = vrapi_Initialize(&initParms);
  if (initResult != VRAPI_INITIALIZE_SUCCESS) {
    exerr << "VRAPI failed to initialize: " << initResult << std::endl;
	}

  std::cout << "OculusMobile_Init 2 " << initResult << std::endl;

  if (oculusMobileContextConstructor.IsEmpty()) {
    oculusMobileContextConstructor.Reset(OculusMobileContext::Initialize());
  }

  Local<Function> localOculusMobileContextConstructor = Nan::New(oculusMobileContextConstructor);
  /* Local<Value> argv[] = {
  };
  Local<Object> oculusMobileContextObj = localOculusMobileContextConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked(); */
  Local<Object> oculusMobileContextObj = localOculusMobileContextConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
  info.GetReturnValue().Set(oculusMobileContextObj);

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

  vrapi_Shutdown();
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

}

Local<Object> makeOculusMobileVr() {
  v8::EscapableHandleScope scope(Isolate::GetCurrent());

  Local<Object> exports = Object::New(Isolate::GetCurrent());

  exports->Set(Nan::New("OculusMobile_Init").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(oculusmobile::OculusMobile_Init)->GetFunction());
  exports->Set(Nan::New("OculusMobile_Shutdown").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(oculusmobile::OculusMobile_Shutdown)->GetFunction());
  exports->Set(Nan::New("OculusMobile_IsHmdPresent").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(oculusmobile::OculusMobile_IsHmdPresent)->GetFunction());

  return scope.Escape(exports);
}

#endif

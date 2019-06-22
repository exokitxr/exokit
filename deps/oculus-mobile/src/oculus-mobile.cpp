#ifdef ANDROID

#include <oculus-mobile.h>
#include <oculus-context.h>
#include <android_native_app_glue.h>

#include <nan.h>
#include <exout>

#include <node.h>
#include <v8.h>

namespace oculusmobile {

Nan::Persistent<v8::Function> oculusMobileContextConstructor;
Nan::Persistent<v8::Object> oculusMobileContext;

NAN_METHOD(OculusMobile_Init) {
  Local<Array> windowHandleArray = Local<Array>::Cast(info[0]);

  if (oculusMobileContextConstructor.IsEmpty()) {
    oculusMobileContextConstructor.Reset(OculusMobileContext::Initialize());
  }

  Local<Function> localOculusMobileContextConstructor = Nan::New(oculusMobileContextConstructor);
  Local<Value> argv[] = {
    windowHandleArray,
  };
  Local<Object> oculusMobileContextObj = localOculusMobileContextConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();
  // Local<Object> oculusMobileContextObj = localOculusMobileContextConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
  oculusMobileContext.Reset(oculusMobileContextObj);

  OculusMobileContext *omc = ObjectWrap::Unwrap<OculusMobileContext>(oculusMobileContextObj);
  omc->RequestPresent();

  info.GetReturnValue().Set(oculusMobileContextObj);
}

NAN_METHOD(OculusMobile_Shutdown) {
  Local<Object> oculusMobileContextObj = Nan::New(oculusMobileContext);
  OculusMobileContext *omc = ObjectWrap::Unwrap<OculusMobileContext>(oculusMobileContextObj);
  omc->Destroy();
  oculusMobileContext.Reset();

  vrapi_Shutdown();
}

NAN_METHOD(OculusMobile_IsHmdPresent) {
  info.GetReturnValue().Set(Nan::New<Boolean>(true));
}

NAN_METHOD(OculusMobile_GetDeviceType) {
  const char* id;
  Local<String> result;
  int deviceType = vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_DEVICE_TYPE);
  if (deviceType >= VRAPI_DEVICE_TYPE_GEARVR_START && deviceType <= VRAPI_DEVICE_TYPE_GEARVR_END) {
    id = "GearVR";
  }
  if (deviceType >= VRAPI_DEVICE_TYPE_OCULUSGO_START && deviceType <= VRAPI_DEVICE_TYPE_OCULUSGO_END) {
    id = "OculusGo";
  }
  if (deviceType >= VRAPI_DEVICE_TYPE_OCULUSQUEST_START && deviceType <= VRAPI_DEVICE_TYPE_OCULUSQUEST_END) {
    id = "OculusQuest";
  }

  info.GetReturnValue().Set(Nan::New<String>(id).ToLocalChecked());
}

}

Local<Object> makeOculusMobileVr() {
  v8::EscapableHandleScope scope(Isolate::GetCurrent());

  Local<Object> exports = Object::New(Isolate::GetCurrent());

  exports->Set(Nan::New("OculusMobile_Init").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(oculusmobile::OculusMobile_Init)).ToLocalChecked());
  exports->Set(Nan::New("OculusMobile_Shutdown").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(oculusmobile::OculusMobile_Shutdown)).ToLocalChecked());
  exports->Set(Nan::New("OculusMobile_IsHmdPresent").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(oculusmobile::OculusMobile_IsHmdPresent)).ToLocalChecked());
  exports->Set(Nan::New("OculusMobile_GetDeviceType").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(oculusmobile::OculusMobile_GetDeviceType)).ToLocalChecked());

  return scope.Escape(exports);
}

#endif

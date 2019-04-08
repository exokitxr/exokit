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
  std::cout << "OculusMobile_Init 1" << std::endl;

  if (oculusMobileContextConstructor.IsEmpty()) {
    oculusMobileContextConstructor.Reset(OculusMobileContext::Initialize());
  }

  Local<Function> localOculusMobileContextConstructor = Nan::New(oculusMobileContextConstructor);
  /* Local<Value> argv[] = {
  };
  Local<Object> oculusMobileContextObj = localOculusMobileContextConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked(); */
  Local<Object> oculusMobileContextObj = localOculusMobileContextConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
  oculusMobileContext.Reset(oculusMobileContextObj);
  info.GetReturnValue().Set(oculusMobileContextObj);
}

NAN_METHOD(OculusMobile_Shutdown) {
  std::cout << "OculusMobile_Shutdown" << std::endl;

  Local<Object> oculusMobileContextObj = Nan::New(oculusMobileContext);
  OculusMobileContext *omc = ObjectWrap::Unwrap<OculusMobileContext>(oculusMobileContextObj);
  omc->Destroy();
  oculusMobileContext.Reset();

  vrapi_Shutdown();
}

NAN_METHOD(OculusMobile_IsHmdPresent) {
  std::cout << "OculusMobile_IsHmdPresent" << std::endl;

  info.GetReturnValue().Set(Nan::New<Boolean>(true));
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

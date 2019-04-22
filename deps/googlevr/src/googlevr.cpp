#ifdef ANDROID

#include <googlevr.h>
#include <googlevr-context.h>
#include <android_native_app_glue.h>

#include <nan.h>
#include <exout>

#include <node.h>
#include <v8.h>

namespace googlevr {

Nan::Persistent<v8::Function> googleVrContextConstructor;
Nan::Persistent<v8::Object> googleVrContext;

NAN_METHOD(GoogleVr_Init) {
  std::cout << "GoogleVr_Init 1" << std::endl;

  Local<Array> windowHandleArray = Local<Array>::Cast(info[0]);

  if (googleVrContextConstructor.IsEmpty()) {
    googleVrContextConstructor.Reset(GoogleVrContext::Initialize());
  }

  Local<Function> localGoogleVrContextConstructor = Nan::New(googleVrContextConstructor);
  Local<Value> argv[] = {
    windowHandleArray,
  };
  Local<Object> googleVrContextObj = localGoogleVrContextConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();
  // Local<Object> googleVrContextObj = localGoogleVrContextConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
  googleVrContext.Reset(googleVrContextObj);

  GoogleVrContext *omc = ObjectWrap::Unwrap<GoogleVrContext>(googleVrContextObj);
  omc->RequestPresent();

  info.GetReturnValue().Set(googleVrContextObj);
}

NAN_METHOD(GoogleVr_Shutdown) {
  std::cout << "GoogleVr_Shutdown" << std::endl;

  Local<Object> googleVrContextObj = Nan::New(googleVrContext);
  GoogleVrContext *omc = ObjectWrap::Unwrap<GoogleVrContext>(googleVrContextObj);
  omc->Destroy();
  googleVrContext.Reset();

  // gvr_Shutdown();
}

NAN_METHOD(GoogleVr_IsHmdPresent) {
  std::cout << "GoogleVr_IsHmdPresent" << std::endl;

  info.GetReturnValue().Set(Nan::New<Boolean>(true));
}

}

Local<Object> makeGoogleVrVr() {
  v8::EscapableHandleScope scope(Isolate::GetCurrent());

  Local<Object> exports = Object::New(Isolate::GetCurrent());

  exports->Set(Nan::New("GoogleVr_Init").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(googlevr::GoogleVr_Init)->GetFunction());
  exports->Set(Nan::New("GoogleVr_Shutdown").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(googlevr::GoogleVr_Shutdown)->GetFunction());
  exports->Set(Nan::New("GoogleVr_IsHmdPresent").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(googlevr::GoogleVr_IsHmdPresent)->GetFunction());

  return scope.Escape(exports);
}

#endif

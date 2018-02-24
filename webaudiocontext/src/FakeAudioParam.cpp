#include <FakeAudioParam.h>

namespace webaudio {

FakeAudioParam::FakeAudioParam() {}
FakeAudioParam::~FakeAudioParam() {}

Handle<Object> FakeAudioParam::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("FakeAudioParam"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetAccessor(proto, JS_STR("defaultValue"), DefaultValueGetter);
  Nan::SetAccessor(proto, JS_STR("maxValue"), MaxValueGetter);
  Nan::SetAccessor(proto, JS_STR("minValue"), MinValueGetter);
  Nan::SetAccessor(proto, JS_STR("value"), ValueGetter, ValueSetter);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_METHOD(FakeAudioParam::New) {
  Nan::HandleScope scope;

  FakeAudioParam *fakeAudioParam = new FakeAudioParam();
  Local<Object> fakeAudioParamObj = info.This();
  fakeAudioParam->Wrap(fakeAudioParamObj);

  info.GetReturnValue().Set(fakeAudioParamObj);
}

NAN_GETTER(FakeAudioParam::DefaultValueGetter) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_INT(0));
}

NAN_GETTER(FakeAudioParam::MaxValueGetter) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_INT(0));
}

NAN_GETTER(FakeAudioParam::MinValueGetter) {
  Nan::HandleScope scope;

  info.GetReturnValue().Set(JS_INT(0));
}

NAN_GETTER(FakeAudioParam::ValueGetter) {
  Nan::HandleScope scope;

  FakeAudioParam *fakeAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(info.This());

  float value = fakeAudioParam->getter();

  info.GetReturnValue().Set(JS_NUM(value));
}

NAN_SETTER(FakeAudioParam::ValueSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber()) {
    FakeAudioParam *fakeAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(info.This());

    float newValue = value->NumberValue();

    fakeAudioParam->setter(newValue);
  } else {
    Nan::ThrowError("value: invalid arguments");
  }
}

}
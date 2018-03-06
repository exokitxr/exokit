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
  Nan::SetMethod(proto, "setValueAtTime", SetValueAtTime);
  Nan::SetMethod(proto, "linearRampToValueAtTime", LinearRampToValueAtTime);
  Nan::SetMethod(proto, "exponentialRampToValueAtTime", ExponentialRampToValueAtTime);
  Nan::SetMethod(proto, "setTargetAtTime", SetTargetAtTime);
  Nan::SetMethod(proto, "setValueCurveAtTime", SetValueCurveAtTime);
  Nan::SetMethod(proto, "cancelScheduledValues", CancelScheduledValues);
  Nan::SetMethod(proto, "cancelAndHoldAtTime", CancelAndHoldAtTime);

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

NAN_METHOD(FakeAudioParam::SetValueAtTime) {
  if (info[0]->IsNumber()) {
    FakeAudioParam *fakeAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(info.This());

    float newValue = info[0]->NumberValue();
    fakeAudioParam->setter(newValue);
  } else {
    Nan::ThrowError("setValueAtTime: invalid arguments");
  }
}

NAN_METHOD(FakeAudioParam::LinearRampToValueAtTime) {
  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    FakeAudioParam *fakeAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(info.This());

    float newValue = info[0]->NumberValue();
    fakeAudioParam->setter(newValue);
  } else {
    Nan::ThrowError("linearRampToValueAtTime: invalid arguments");
  }
}

NAN_METHOD(FakeAudioParam::ExponentialRampToValueAtTime) {
  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    FakeAudioParam *fakeAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(info.This());

    float newValue = info[0]->NumberValue();
    fakeAudioParam->setter(newValue);
  } else {
    Nan::ThrowError("exponentialRampToValueAtTime: invalid arguments");
  }
}

NAN_METHOD(FakeAudioParam::SetTargetAtTime) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    FakeAudioParam *fakeAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(info.This());

    float newValue = info[0]->NumberValue();
    fakeAudioParam->setter(newValue);
  } else {
    Nan::ThrowError("setTargetAtTime: invalid arguments");
  }
}

NAN_METHOD(FakeAudioParam::SetValueCurveAtTime) {
  if (info[0]->IsFloat32Array() && info[1]->IsNumber() && info[2]->IsNumber()) {
    FakeAudioParam *fakeAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(info.This());

    Local<Float32Array> curveValue = Local<Float32Array>::Cast(info[0]);
    float newValue = curveValue->Length() == 0 ? 0 : curveValue->Get(curveValue->Length() - 1)->NumberValue();
    fakeAudioParam->setter(newValue);
  } else {
    Nan::ThrowError("setValueCurveAtTime: invalid arguments");
  }
}

NAN_METHOD(FakeAudioParam::CancelScheduledValues) {
  if (info[0]->IsNumber()) {
    // nothing
  } else {
    Nan::ThrowError("cancelScheduledValues: invalid arguments");
  }
}

NAN_METHOD(FakeAudioParam::CancelAndHoldAtTime) {
  if (info[0]->IsNumber()) {
    // nothing
  } else {
    Nan::ThrowError("cancelAndHoldAtTime: invalid arguments");
  }
}

}

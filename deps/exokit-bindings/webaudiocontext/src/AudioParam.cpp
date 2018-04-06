#include <AudioParam.h>

namespace webaudio {

AudioParam::AudioParam() {}
AudioParam::~AudioParam() {}

Handle<Object> AudioParam::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioParam"));

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

NAN_METHOD(AudioParam::New) {
  Nan::HandleScope scope;

  AudioParam *audioParam = new AudioParam();
  Local<Object> audioParamObj = info.This();
  audioParam->Wrap(audioParamObj);

  info.GetReturnValue().Set(audioParamObj);
}

NAN_GETTER(AudioParam::DefaultValueGetter) {
  Nan::HandleScope scope;

  AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

  float defaultValue = audioParam->audioParam->defaultValue();

  info.GetReturnValue().Set(JS_NUM(defaultValue));
}

NAN_GETTER(AudioParam::MaxValueGetter) {
  Nan::HandleScope scope;

  AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

  float maxValue = audioParam->audioParam->maxValue();

  info.GetReturnValue().Set(JS_NUM(maxValue));
}

NAN_GETTER(AudioParam::MinValueGetter) {
  Nan::HandleScope scope;

  AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

  float minValue = audioParam->audioParam->minValue();

  info.GetReturnValue().Set(JS_NUM(minValue));
}

NAN_GETTER(AudioParam::ValueGetter) {
  Nan::HandleScope scope;

  AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

  float value;
  {
    lab::ContextRenderLock lock(getDefaultAudioContext(), "AudioParam::ValueGetter");
    value = audioParam->audioParam->value(lock);
  }

  info.GetReturnValue().Set(JS_NUM(value));
}

NAN_SETTER(AudioParam::ValueSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber()) {
    AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

    float newValue = value->NumberValue();
    audioParam->audioParam->setValue(newValue);
  } else {
    Nan::ThrowError("setValue: invalid arguments");
  }
}

NAN_METHOD(AudioParam::SetValueAtTime) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

    float newValue = info[0]->NumberValue();
    float startTime = info[1]->NumberValue();
    audioParam->audioParam->setValueAtTime(newValue, startTime);
  } else {
    Nan::ThrowError("setValueAtTime: invalid arguments");
  }
}

NAN_METHOD(AudioParam::LinearRampToValueAtTime) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

    float newValue = info[0]->NumberValue();
    float startTime = info[1]->NumberValue();
    audioParam->audioParam->linearRampToValueAtTime(newValue, startTime);
  } else {
    Nan::ThrowError("linearRampToValueAtTime: invalid arguments");
  }
}

NAN_METHOD(AudioParam::ExponentialRampToValueAtTime) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

    float newValue = info[0]->NumberValue();
    float startTime = info[1]->NumberValue();
    audioParam->audioParam->exponentialRampToValueAtTime(newValue, startTime);
  } else {
    Nan::ThrowError("exponentialRampToValueAtTime: invalid arguments");
  }
}

NAN_METHOD(AudioParam::SetTargetAtTime) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

    float target = info[0]->NumberValue();
    float time = info[1]->NumberValue();
    float timeConstant = info[2]->NumberValue();
    audioParam->audioParam->setTargetAtTime(target, time, timeConstant);
  } else {
    Nan::ThrowError("setTargetAtTime: invalid arguments");
  }
}

NAN_METHOD(AudioParam::SetValueCurveAtTime) {
  Nan::HandleScope scope;

  if (info[0]->IsFloat32Array() && info[1]->IsNumber() && info[2]->IsNumber()) {
    AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

    Local<Float32Array> curveValue = Local<Float32Array>::Cast(info[0]);
    size_t numCurves = curveValue->Length();
    vector<float> curve(numCurves);
    for (size_t i = 0; i < numCurves; i++) {
      curve[i] = curveValue->Get(i)->NumberValue();
    }
    float time = info[1]->NumberValue();
    float duration = info[2]->NumberValue();
    audioParam->audioParam->setValueCurveAtTime(curve, time, duration);
  } else {
    Nan::ThrowError("setValueCurveAtTime: invalid arguments");
  }
}

NAN_METHOD(AudioParam::CancelScheduledValues) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber()) {
    AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

    float startTime = info[0]->NumberValue();
    audioParam->audioParam->cancelScheduledValues(startTime);
  } else {
    Nan::ThrowError("cancelScheduledValues: invalid arguments");
  }
}

NAN_METHOD(AudioParam::CancelAndHoldAtTime) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber()) {
    AudioParam *audioParam = ObjectWrap::Unwrap<AudioParam>(info.This());

    float cancelTime = info[0]->NumberValue();
    audioParam->audioParam->cancelScheduledValues(cancelTime); // TODO: should be cancelAndHoldAtTime
  } else {
    Nan::ThrowError("cancelAndHoldAtTime: invalid arguments");
  }
}

}

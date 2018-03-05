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
    Nan::ThrowError("value: invalid arguments");
  }
}

}

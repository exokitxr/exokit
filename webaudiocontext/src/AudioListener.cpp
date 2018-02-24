#include <AudioListener.h>

namespace webaudio {

AudioListener::AudioListener() {}

AudioListener::~AudioListener() {}

Handle<Object> AudioListener::Initialize(Isolate *isolate, Local<Value> fakeAudioParamCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioListener"));

  // prototype
  // Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Local<Function> ctorFn = ctor->GetFunction();
  
  ctorFn->Set(JS_STR("FakeAudioParam"), fakeAudioParamCons);

  return scope.Escape(ctorFn);
}

NAN_METHOD(AudioListener::New) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[0]);
    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

    AudioListener *audioListener = new AudioListener();
    Local<Object> audioListenerObj = info.This();
    audioListener->Wrap(audioListenerObj);
    
    lab::AudioListener *labAudioListener = &audioContext->audioContext->listener();
    audioListener->audioListener = labAudioListener;
    
    Local<Function> fakeAudioParamConstructor = Local<Function>::Cast(audioListenerObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("FakeAudioParam")));

    Local<Object> positionXAudioParamObj = fakeAudioParamConstructor->NewInstance(0, nullptr);
    FakeAudioParam *positionXAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(positionXAudioParamObj);
    positionXAudioParam->getter = [=]() { return labAudioListener->position().x; };
    positionXAudioParam->setter = [=](float x) { lab::FloatPoint3D position = labAudioListener->position(); position.x = x; labAudioListener->setPosition(position); };
    audioListenerObj->Set(JS_STR("positionX"), positionXAudioParamObj);

    Local<Object> positionYAudioParamObj = fakeAudioParamConstructor->NewInstance(0, nullptr);
    FakeAudioParam *positionYAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(positionYAudioParamObj);
    positionYAudioParam->getter = [=]() { return labAudioListener->position().y; };
    positionYAudioParam->setter = [=](float y) { lab::FloatPoint3D position = labAudioListener->position(); position.y = y; labAudioListener->setPosition(position); };
    audioListenerObj->Set(JS_STR("positionY"), positionYAudioParamObj);

    Local<Object> positionZAudioParamObj = fakeAudioParamConstructor->NewInstance(0, nullptr);
    FakeAudioParam *positionZAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(positionZAudioParamObj);
    positionZAudioParam->getter = [=]() { return labAudioListener->position().z; };
    positionYAudioParam->setter = [=](float z) { lab::FloatPoint3D position = labAudioListener->position(); position.z = z; labAudioListener->setPosition(position); };
    audioListenerObj->Set(JS_STR("positionZ"), positionZAudioParamObj);

    Local<Object> forwardXAudioParamObj = fakeAudioParamConstructor->NewInstance(0, nullptr);
    FakeAudioParam *forwardXAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(forwardXAudioParamObj);
    forwardXAudioParam->getter = [=]() { return labAudioListener->orientation().x; };
    forwardXAudioParam->setter = [=](float x) { lab::FloatPoint3D orientation = labAudioListener->orientation(); orientation.x = x; labAudioListener->setOrientation(orientation); };
    audioListenerObj->Set(JS_STR("forwardX"), forwardXAudioParamObj);

    Local<Object> forwardYAudioParamObj = fakeAudioParamConstructor->NewInstance(0, nullptr);
    FakeAudioParam *forwardYAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(forwardYAudioParamObj);
    forwardYAudioParam->getter = [=]() { return labAudioListener->orientation().y; };
    forwardYAudioParam->setter = [=](float y) { lab::FloatPoint3D orientation = labAudioListener->orientation(); orientation.y = y; labAudioListener->setOrientation(orientation); };
    audioListenerObj->Set(JS_STR("forwardY"), forwardYAudioParamObj);

    Local<Object> forwardZAudioParamObj = fakeAudioParamConstructor->NewInstance(0, nullptr);
    FakeAudioParam *forwardZAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(forwardZAudioParamObj);
    forwardZAudioParam->getter = [=]() { return labAudioListener->orientation().z; };
    forwardZAudioParam->setter = [=](float z) { lab::FloatPoint3D orientation = labAudioListener->orientation(); orientation.z = z; labAudioListener->setOrientation(orientation); };
    audioListenerObj->Set(JS_STR("forwardZ"), forwardZAudioParamObj);

    Local<Object> upXAudioParamObj = fakeAudioParamConstructor->NewInstance(0, nullptr);
    FakeAudioParam *upXAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(upXAudioParamObj);
    upXAudioParam->getter = [=]() { return labAudioListener->upVector().x; };
    upXAudioParam->setter = [=](float x) { lab::FloatPoint3D up = labAudioListener->upVector(); up.x = x; labAudioListener->setUpVector(up); };
    audioListenerObj->Set(JS_STR("upX"), upXAudioParamObj);

    Local<Object> upYAudioParamObj = fakeAudioParamConstructor->NewInstance(0, nullptr);
    FakeAudioParam *upYAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(upYAudioParamObj);
    upYAudioParam->getter = [=]() { return labAudioListener->upVector().y; };
    upYAudioParam->setter = [=](float y) { lab::FloatPoint3D up = labAudioListener->upVector(); up.y = y; labAudioListener->setUpVector(up); };
    audioListenerObj->Set(JS_STR("upY"), upYAudioParamObj);

    Local<Object> upZAudioParamObj = fakeAudioParamConstructor->NewInstance(0, nullptr);
    FakeAudioParam *upZAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(upZAudioParamObj);
    upZAudioParam->getter = [=]() { return labAudioListener->upVector().z; };
    upZAudioParam->setter = [=](float z) { lab::FloatPoint3D up = labAudioListener->upVector(); up.z = z; labAudioListener->setUpVector(up); };
    audioListenerObj->Set(JS_STR("upZ"), upZAudioParamObj);

    info.GetReturnValue().Set(audioListenerObj);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(AudioListener::SetPosition) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    AudioListener *audioListener = ObjectWrap::Unwrap<AudioListener>(info.This());

    float x = info[0]->NumberValue();
    float y = info[1]->NumberValue();
    float z = info[2]->NumberValue();

    audioListener->audioListener->setPosition(lab::FloatPoint3D{x, y, z});
  } else {
    Nan::ThrowError("AnalyserNode::GetFloatFrequencyData: invalid arguments");
  }
}

NAN_METHOD(AudioListener::SetOrientation) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    AudioListener *audioListener = ObjectWrap::Unwrap<AudioListener>(info.This());

    float x = info[0]->NumberValue();
    float y = info[1]->NumberValue();
    float z = info[2]->NumberValue();

    audioListener->audioListener->setOrientation(lab::FloatPoint3D{x, y, z});
  } else {
    Nan::ThrowError("AnalyserNode::GetFloatFrequencyData: invalid arguments");
  }
}

}
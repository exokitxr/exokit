#include <OscillatorNode.h>

namespace webaudio {

OscillatorNode::OscillatorNode() {}

OscillatorNode::~OscillatorNode() {}

Handle<Object> OscillatorNode::Initialize(Isolate *isolate, Local<Value> audioParamCons) {
  Nan::EscapableHandleScope scope;
  
  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("OscillatorNode"));
  
  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  OscillatorNode::InitializePrototype(proto);
  
  Local<Function> ctorFn = ctor->GetFunction();
  
  ctorFn->Set(JS_STR("AudioParam"), audioParamCons);

  return scope.Escape(ctorFn);
}

void OscillatorNode::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetMethod(proto, "start", Start);
  Nan::SetMethod(proto, "stop", Stop);
}

NAN_METHOD(OscillatorNode::New) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[0]);

    OscillatorNode *oscillatorNode = new OscillatorNode();
    Local<Object> oscillatorNodeObj = info.This();
    oscillatorNode->Wrap(oscillatorNodeObj);

    double sampleRate = getDefaultAudioContext()->sampleRate();
    oscillatorNode->context.Reset(audioContextObj);
    oscillatorNode->audioNode = make_shared<lab::OscillatorNode>(sampleRate);

    Local<Function> audioParamConstructor = Local<Function>::Cast(oscillatorNodeObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioParam")));

    Local<Object> frequencyAudioParamObj = audioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
    AudioParam *frequencyAudioParam = ObjectWrap::Unwrap<AudioParam>(frequencyAudioParamObj);
    frequencyAudioParam->audioParam = (*(shared_ptr<lab::OscillatorNode> *)(&oscillatorNode->audioNode))->frequency();
    oscillatorNodeObj->Set(JS_STR("frequency"), frequencyAudioParamObj);

    Local<Object> detuneAudioParamObj = audioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
    AudioParam *detuneAudioParam = ObjectWrap::Unwrap<AudioParam>(detuneAudioParamObj);
    detuneAudioParam->audioParam = (*(shared_ptr<lab::OscillatorNode> *)(&oscillatorNode->audioNode))->detune();
    oscillatorNodeObj->Set(JS_STR("detune"), detuneAudioParamObj);

    info.GetReturnValue().Set(oscillatorNodeObj);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(OscillatorNode::Start) {
  Nan::HandleScope scope;

  OscillatorNode *oscillatorNode = ObjectWrap::Unwrap<OscillatorNode>(info.This());
  ((lab::FinishableSourceNode *)oscillatorNode->audioNode.get())->start(0);
}
NAN_METHOD(OscillatorNode::Stop) {
  Nan::HandleScope scope;

  OscillatorNode *oscillatorNode = ObjectWrap::Unwrap<OscillatorNode>(info.This());
  ((lab::FinishableSourceNode *)oscillatorNode->audioNode.get())->stop(0);
}

}

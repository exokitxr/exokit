#include <OscillatorNode.h>

namespace webaudio {

OscillatorNode::OscillatorNode() {}

OscillatorNode::~OscillatorNode() {}

Local<Object> OscillatorNode::Initialize(Isolate *isolate, Local<Value> audioParamCons) {
  Nan::EscapableHandleScope scope;
  
  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("OscillatorNode"));
  
  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  OscillatorNode::InitializePrototype(proto);
  
  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();
  
  ctorFn->Set(JS_STR("AudioParam"), audioParamCons);

  return scope.Escape(ctorFn);
}

void OscillatorNode::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetMethod(proto, "start", Start);
  Nan::SetMethod(proto, "stop", Stop);
}

NAN_METHOD(OscillatorNode::New) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && JS_OBJ(JS_OBJ(info[0])->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[0]);

    OscillatorNode *oscillatorNode = new OscillatorNode();
    Local<Object> oscillatorNodeObj = info.This();
    oscillatorNode->Wrap(oscillatorNodeObj);

    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);
    oscillatorNode->audioNode = make_shared<lab::OscillatorNode>(audioContext->audioContext->sampleRate());

    oscillatorNode->context.Reset(audioContextObj);

    Local<Function> audioParamConstructor = Local<Function>::Cast(JS_OBJ(oscillatorNodeObj->Get(JS_STR("constructor")))->Get(JS_STR("AudioParam")));
    Local<Value> args[] = {
      audioContextObj,
    };

    Local<Object> frequencyAudioParamObj = audioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(args)/sizeof(args[0]), args).ToLocalChecked();
    AudioParam *frequencyAudioParam = ObjectWrap::Unwrap<AudioParam>(frequencyAudioParamObj);
    frequencyAudioParam->audioParam = (*(shared_ptr<lab::OscillatorNode> *)(&oscillatorNode->audioNode))->frequency();
    oscillatorNodeObj->Set(JS_STR("frequency"), frequencyAudioParamObj);

    Local<Object> detuneAudioParamObj = audioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(args)/sizeof(args[0]), args).ToLocalChecked();
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

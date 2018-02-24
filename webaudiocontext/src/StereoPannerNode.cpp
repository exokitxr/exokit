#include <StereoPannerNode.h>

namespace webaudio {

StereoPannerNode::StereoPannerNode() {}

StereoPannerNode::~StereoPannerNode() {}

Handle<Object> StereoPannerNode::Initialize(Isolate *isolate, Local<Value> audioParamCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("StereoPannerNode"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  StereoPannerNode::InitializePrototype(proto);

  Local<Function> ctorFn = ctor->GetFunction();

  ctorFn->Set(JS_STR("AudioParam"), audioParamCons);

  return scope.Escape(ctorFn);
}

void StereoPannerNode::InitializePrototype(Local<ObjectTemplate> proto) {
  // nothing
}

NAN_METHOD(StereoPannerNode::New) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[0]);

    StereoPannerNode *stereoPannerNode = new StereoPannerNode();
    Local<Object> stereoPannerNodeObj = info.This();
    stereoPannerNode->Wrap(stereoPannerNodeObj);

    shared_ptr<lab::StereoPannerNode> labStereoPannerNode = make_shared<lab::StereoPannerNode>(defaultAudioContext->sampleRate());
    
    stereoPannerNode->context.Reset(audioContextObj);
    stereoPannerNode->audioNode = labStereoPannerNode;

    Local<Function> audioParamConstructor = Local<Function>::Cast(stereoPannerNodeObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioParam")));

    Local<Object> panAudioParamObj = audioParamConstructor->NewInstance(0, nullptr);
    AudioParam *panAudioParam = ObjectWrap::Unwrap<AudioParam>(panAudioParamObj);
    panAudioParam->audioParam = (*(shared_ptr<lab::StereoPannerNode> *)(&stereoPannerNode->audioNode))->pan();
    stereoPannerNodeObj->Set(JS_STR("pan"), panAudioParamObj);

    info.GetReturnValue().Set(stereoPannerNodeObj);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

}
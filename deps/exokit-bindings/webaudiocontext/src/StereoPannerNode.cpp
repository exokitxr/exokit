#include <StereoPannerNode.h>

namespace webaudio {

StereoPannerNode::StereoPannerNode() {}

StereoPannerNode::~StereoPannerNode() {}

Local<Object> StereoPannerNode::Initialize(Isolate *isolate, Local<Value> audioParamCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("StereoPannerNode"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  StereoPannerNode::InitializePrototype(proto);

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();

  ctorFn->Set(JS_STR("AudioParam"), audioParamCons);

  return scope.Escape(ctorFn);
}

void StereoPannerNode::InitializePrototype(Local<ObjectTemplate> proto) {
  // nothing
}

NAN_METHOD(StereoPannerNode::New) {
  // Nan::HandleScope scope;

  if (info[0]->IsObject() && JS_OBJ(JS_OBJ(info[0])->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[0]);

    StereoPannerNode *stereoPannerNode = new StereoPannerNode();
    Local<Object> stereoPannerNodeObj = info.This();
    stereoPannerNode->Wrap(stereoPannerNodeObj);

    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);
    shared_ptr<lab::StereoPannerNode> labStereoPannerNode = make_shared<lab::StereoPannerNode>(audioContext->audioContext->sampleRate());
    
    stereoPannerNode->context.Reset(audioContextObj);
    stereoPannerNode->audioNode = labStereoPannerNode;

    Local<Function> audioParamConstructor = Local<Function>::Cast(JS_OBJ(stereoPannerNodeObj->Get(JS_STR("constructor")))->Get(JS_STR("AudioParam")));
    Local<Value> args[] = {
      audioContextObj,
    };

    Local<Object> panAudioParamObj = audioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(args)/sizeof(args[0]), args).ToLocalChecked();
    AudioParam *panAudioParam = ObjectWrap::Unwrap<AudioParam>(panAudioParamObj);
    panAudioParam->audioParam = (*(shared_ptr<lab::StereoPannerNode> *)(&stereoPannerNode->audioNode))->pan();
    stereoPannerNodeObj->Set(JS_STR("pan"), panAudioParamObj);

    info.GetReturnValue().Set(stereoPannerNodeObj);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

}

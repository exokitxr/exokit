#include <GainNode.h>
#include <AudioContext.h>

namespace webaudio {

GainNode::GainNode() {}

GainNode::~GainNode() {}

Local<Object> GainNode::Initialize(Isolate *isolate, Local<Value> audioParamCons) {
  Nan::EscapableHandleScope scope;
  
  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("GainNode"));
  
  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  GainNode::InitializePrototype(proto);
  
  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();
  
  ctorFn->Set(JS_STR("AudioParam"), audioParamCons);

  return scope.Escape(ctorFn);
}

void GainNode::InitializePrototype(Local<ObjectTemplate> proto) {
  // nothing
}

NAN_METHOD(GainNode::New) {
  // Nan::HandleScope scope;

  if (info[0]->IsObject() && JS_OBJ(JS_OBJ(info[0])->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[0]);

    GainNode *gainNode = new GainNode();
    Local<Object> gainNodeObj = info.This();
    gainNode->Wrap(gainNodeObj);

    gainNode->context.Reset(audioContextObj);
    gainNode->audioNode = make_shared<lab::GainNode>();

    Local<Function> audioParamConstructor = Local<Function>::Cast(JS_OBJ(gainNodeObj->Get(JS_STR("constructor")))->Get(JS_STR("AudioParam")));
    Local<Value> args[] = {
      audioContextObj,
    };

    Local<Object> gainAudioParamObj = audioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(args)/sizeof(args[0]), args).ToLocalChecked();
    AudioParam *gainAudioParam = ObjectWrap::Unwrap<AudioParam>(gainAudioParamObj);
    gainAudioParam->audioParam = (*(shared_ptr<lab::GainNode> *)(&gainNode->audioNode))->gain();
    gainNodeObj->Set(JS_STR("gain"), gainAudioParamObj);

    info.GetReturnValue().Set(gainNodeObj);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

}

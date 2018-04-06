#include <AudioDestinationNode.h>

namespace webaudio {

AudioDestinationNode::AudioDestinationNode() {}

AudioDestinationNode::~AudioDestinationNode() {}

Handle<Object> AudioDestinationNode::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;
  
  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioDestinationNode"));
  
  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  AudioDestinationNode::InitializePrototype(proto);
  
  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

void AudioDestinationNode::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetAccessor(proto, JS_STR("maxChannelCount"), MaxChannelCountGetter);
}

NAN_METHOD(AudioDestinationNode::New) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[0]);
    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);
    lab::AudioContext *labAudioContext = audioContext->audioContext;
    
    AudioDestinationNode *audioDestinationNode = new AudioDestinationNode();
    Local<Object> audioDestinationNodeObj = info.This();
    audioDestinationNode->Wrap(audioDestinationNodeObj);

    audioDestinationNode->context.Reset(audioContextObj);
    audioDestinationNode->audioNode = labAudioContext->destination();

    info.GetReturnValue().Set(audioDestinationNodeObj);
  } else {
    Nan::ThrowError("AudioDestinationNode: invalid arguments");
  }
}

NAN_GETTER(AudioDestinationNode::MaxChannelCountGetter) {
  Nan::HandleScope scope;
  
  info.GetReturnValue().Set(JS_INT(getDefaultAudioContext()->maxNumberOfChannels));
}

}

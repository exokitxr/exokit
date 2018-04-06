#include <AudioSourceNode.h>

namespace webaudio {

AudioSourceNode::AudioSourceNode() {}

AudioSourceNode::~AudioSourceNode() {}

Handle<Object> AudioSourceNode::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioSourceNode"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  AudioSourceNode::InitializePrototype(proto);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

void AudioSourceNode::InitializePrototype(Local<ObjectTemplate> proto) {
  // nothing
}

NAN_METHOD(AudioSourceNode::New) {
  Nan::HandleScope scope;

  if (info[1]->IsObject() && info[1]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[1]);

    if (info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("HTMLAudioElement"))) {
      Local<Object> htmlAudioElement = Local<Object>::Cast(info[0]);
      Local<Value> audioValue = htmlAudioElement->Get(JS_STR("audio"));

      if (audioValue->BooleanValue() && audioValue->IsObject() && audioValue->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("Audio"))) {
        Audio *audio = ObjectWrap::Unwrap<Audio>(Local<Object>::Cast(audioValue));

        AudioSourceNode *audioSourceNode = new AudioSourceNode();
        Local<Object> audioSourceNodeObj = info.This();
        audioSourceNode->Wrap(audioSourceNodeObj);

        audioSourceNode->context.Reset(audioContextObj);
        audioSourceNode->audioNode = audio->audioNode;

        info.GetReturnValue().Set(audioSourceNodeObj);
      } else {
        Nan::ThrowError("AudioSourceNode: invalid audio element state");
      }
    } else if (info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("MicrophoneMediaStream"))) {
      Local<Object> microphoneMediaStreamObj = Local<Object>::Cast(info[0]);
      MicrophoneMediaStream *microphoneMediaStream = ObjectWrap::Unwrap<MicrophoneMediaStream>(Local<Object>::Cast(microphoneMediaStreamObj));

      AudioSourceNode *audioSourceNode = new AudioSourceNode();
      Local<Object> audioSourceNodeObj = info.This();
      audioSourceNode->Wrap(audioSourceNodeObj);

      audioSourceNode->context.Reset(audioContextObj);
      audioSourceNode->audioNode = microphoneMediaStream->audioNode;

      info.GetReturnValue().Set(audioSourceNodeObj);
    } else {
      Nan::ThrowError("AudioSourceNode: invalid media element");
    }
  } else {
    Nan::ThrowError("AudioSourceNode: invalid audio context");
  }
}

}

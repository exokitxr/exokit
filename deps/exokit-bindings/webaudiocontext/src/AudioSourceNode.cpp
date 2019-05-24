#include <AudioSourceNode.h>

namespace webaudio {

AudioSourceNode::AudioSourceNode() {}

AudioSourceNode::~AudioSourceNode() {}

Local<Object> AudioSourceNode::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioSourceNode"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  AudioSourceNode::InitializePrototype(proto);

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();

  return scope.Escape(ctorFn);
}

void AudioSourceNode::InitializePrototype(Local<ObjectTemplate> proto) {
  // nothing
}

NAN_METHOD(AudioSourceNode::New) {
  // Nan::HandleScope scope;

  if (info[1]->IsObject() && JS_OBJ(JS_OBJ(info[1])->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[1]);

    if (info[0]->IsObject() && JS_OBJ(JS_OBJ(info[0])->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("HTMLAudioElement"))) {
      Local<Object> htmlAudioElement = Local<Object>::Cast(info[0]);
      Local<Value> audioValue = htmlAudioElement->Get(JS_STR("audio"));

      if (TO_BOOL(audioValue) && audioValue->IsObject() && JS_OBJ(JS_OBJ(audioValue)->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("Audio"))) {
        Audio *audio = ObjectWrap::Unwrap<Audio>(Local<Object>::Cast(audioValue));

        AudioSourceNode *audioSourceNode = new AudioSourceNode();
        Local<Object> audioSourceNodeObj = info.This();
        audioSourceNode->Wrap(audioSourceNodeObj);

        audioSourceNode->context.Reset(audioContextObj);
        audioSourceNode->audioNode = audio->audioNode;

        AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(Local<Object>::Cast(audioContextObj));
        audio->Reparent(audioContext);

        info.GetReturnValue().Set(audioSourceNodeObj);
      } else {
        Nan::ThrowError("AudioSourceNode: invalid audio element state");
      }
    } else if (info[0]->IsObject() && JS_OBJ(JS_OBJ(info[0])->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("MicrophoneMediaStream"))) {
      AudioSourceNode *audioSourceNode = new AudioSourceNode();
      Local<Object> audioSourceNodeObj = info.This();
      audioSourceNode->Wrap(audioSourceNodeObj);
      
      audioSourceNode->context.Reset(audioContextObj);
  
      AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(Local<Object>::Cast(audioContextObj));
      {
        lab::ContextRenderLock r(audioContext->audioContext.get(), "AudioSourceNode::New");
        audioSourceNode->audioNode = lab::MakeHardwareSourceNode(r);
      }
      
      info.GetReturnValue().Set(audioSourceNodeObj);

      /* Local<Object> microphoneMediaStreamObj = Local<Object>::Cast(info[0]);
      MicrophoneMediaStream *microphoneMediaStream = ObjectWrap::Unwrap<MicrophoneMediaStream>(Local<Object>::Cast(microphoneMediaStreamObj));

      if (microphoneMediaStream->audioNode) {
        AudioSourceNode *audioSourceNode = new AudioSourceNode();
        Local<Object> audioSourceNodeObj = info.This();
        audioSourceNode->Wrap(audioSourceNodeObj);

        audioSourceNode->context.Reset(audioContextObj);
        audioSourceNode->audioNode = microphoneMediaStream->audioNode;

        info.GetReturnValue().Set(audioSourceNodeObj);
      } else {
        Nan::ThrowError("AudioSourceNode: media stream is not live");
      } */
    } else {
      Nan::ThrowError("AudioSourceNode: invalid media element");
    }
  } else {
    Nan::ThrowError("AudioSourceNode: invalid audio context");
  }
}

}

#include <AudioSourceNode.h>

namespace webaudio {

MicrophoneMediaStream::MicrophoneMediaStream() {}

MicrophoneMediaStream::~MicrophoneMediaStream() {}

Handle<Object> MicrophoneMediaStream::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("MicrophoneMediaStream"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  AudioSourceNode::InitializePrototype(proto);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

void MicrophoneMediaStream::InitializePrototype(Local<ObjectTemplate> proto) {
  // nothing
}

NAN_METHOD(MicrophoneMediaStream::New) {
  Nan::HandleScope scope;

  MicrophoneMediaStream *microphoneMediaStream = new MicrophoneMediaStream();
  Local<Object> microphoneMediaStreamObj = info.This();
  microphoneMediaStream->Wrap(microphoneMediaStreamObj);

  {
    lab::ContextRenderLock r(getDefaultAudioContext(), "MicrophoneMediaStream::New");
    microphoneMediaStream->audioNode = lab::MakeHardwareSourceNode(r);
  }

  info.GetReturnValue().Set(microphoneMediaStreamObj);
}

}

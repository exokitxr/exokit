#include <MicrophoneMediaStream.h>

namespace webaudio {

MicrophoneMediaStream::MicrophoneMediaStream() : tracks(Nan::New<Array>(0)) {}

MicrophoneMediaStream::~MicrophoneMediaStream() {}

Handle<Object> MicrophoneMediaStream::Initialize(Isolate *isolate, Local<Value> mediaStreamTrackCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("MicrophoneMediaStream"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  AudioSourceNode::InitializePrototype(proto);
  MicrophoneMediaStream::InitializePrototype(proto);

  Local<Function> ctorFn = ctor->GetFunction();
  ctorFn->Set(JS_STR("MediaStreamTrack"), mediaStreamTrackCons);

  return scope.Escape(ctorFn);
}

void MicrophoneMediaStream::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetMethod(proto, "getTracks", GetTracks);
}

NAN_METHOD(MicrophoneMediaStream::New) {
  Nan::HandleScope scope;

  MicrophoneMediaStream *microphoneMediaStream = new MicrophoneMediaStream();
  Local<Object> microphoneMediaStreamObj = info.This();
  microphoneMediaStream->Wrap(microphoneMediaStreamObj);

  Local<Function> mediaStreamTrackConstructor = Local<Function>::Cast(microphoneMediaStreamObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("MediaStreamTrack")));
  Local<Value> argv[] = {
    microphoneMediaStreamObj,
  };
  Local<Object> mediaStreamTrackObj = mediaStreamTrackConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();
  Local<Array> localTracks = Nan::New(microphoneMediaStream->tracks);
  localTracks->Set(0, mediaStreamTrackObj);

  {
    lab::ContextRenderLock r(getDefaultAudioContext(), "MicrophoneMediaStream::New");
    microphoneMediaStream->audioNode = lab::MakeHardwareSourceNode(r);
  }

  info.GetReturnValue().Set(microphoneMediaStreamObj);
}

NAN_METHOD(MicrophoneMediaStream::GetTracks) {
  Nan::HandleScope scope;
  
  MicrophoneMediaStream *microphoneMediaStream = ObjectWrap::Unwrap<MicrophoneMediaStream>(info.This());
  Local<Array> localTracks = Nan::New(microphoneMediaStream->tracks);
  info.GetReturnValue().Set(localTracks);
}

}

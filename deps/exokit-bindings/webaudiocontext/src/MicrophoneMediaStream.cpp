#include <MicrophoneMediaStream.h>

namespace webaudio {

MicrophoneMediaStream::MicrophoneMediaStream() : tracks(Nan::New<Array>(0)) {}

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
  MicrophoneMediaStream::InitializePrototype(proto);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

void MicrophoneMediaStream::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetMethod(proto, "getTracks", GetTracks);
}

NAN_METHOD(MicrophoneMediaStream::New) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("MediaStreamTrack"))) {
    MicrophoneMediaStream *microphoneMediaStream = new MicrophoneMediaStream();
    Local<Object> microphoneMediaStreamObj = info.This();
    microphoneMediaStream->Wrap(microphoneMediaStreamObj);

    Local<Object> mediaStreamTrackObj = Local<Object>::Cast(info[0]);
    mediaStreamTrackObj->Set(JS_STR("stop"), Nan::New<Function>(MicrophoneMediaStream::Stop));
    mediaStreamTrackObj->Set(JS_STR("mediaStream"), microphoneMediaStreamObj);
    Local<Array> localTracks = Nan::New(microphoneMediaStream->tracks);
    localTracks->Set(0, mediaStreamTrackObj);

    {
      lab::ContextRenderLock r(getDefaultAudioContext(), "MicrophoneMediaStream::New");
      microphoneMediaStream->audioNode = lab::MakeHardwareSourceNode(r);
    }

    info.GetReturnValue().Set(microphoneMediaStreamObj);
  } else {
    Nan::ThrowError("MicrophoneMediaStream: invalid arguments");
  }
}

NAN_METHOD(MicrophoneMediaStream::GetTracks) {
  Nan::HandleScope scope;
  
  MicrophoneMediaStream *microphoneMediaStream = ObjectWrap::Unwrap<MicrophoneMediaStream>(info.This());
  Local<Array> localTracks = Nan::New(microphoneMediaStream->tracks);
  info.GetReturnValue().Set(localTracks);
}

NAN_METHOD(MicrophoneMediaStream::Stop) {
  if (info.This()->IsObject() && info.This()->IsObject() && info.This()->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("MediaStreamTrack"))) {
    Local<Object> mediaStreamTrackObj = Local<Object>::Cast(info.This());
    Local<Object> mediaStreamObj = Local<Object>::Cast(mediaStreamTrackObj->Get(JS_STR("mediaStream")));
    MediaStream *mediaStream = ObjectWrap::Unwrap<MediaStream>(mediaStreamObj);
    mediaStream->audioNode.reset();

    Local<Object> mediaStreamTrackPrototype = Local<Object>::Cast(mediaStreamTrackObj->GetPrototype());
    Local<Function> stopFn = Local<Function>::Cast(mediaStreamTrackPrototype->Get(JS_STR("stop")));
    stopFn->Call(mediaStreamTrackObj, 0, nullptr);
  } else {
    Nan::ThrowError("MicrophoneMediaStream::Stop: invalid arguments");
  }
}

}

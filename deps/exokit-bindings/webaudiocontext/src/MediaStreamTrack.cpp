#include <MediaStreamTrack.h>

namespace webaudio {

MediaStreamTrack::MediaStreamTrack(MediaStream *mediaStream) : mediaStream(mediaStream), live(true), muted(false) {}

MediaStreamTrack::~MediaStreamTrack() {}

Handle<Object> MediaStreamTrack::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("MediaStreamTrack"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  MediaStreamTrack::InitializePrototype(proto);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

void MediaStreamTrack::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetAccessor(proto, JS_STR("enabled"), EnabledGetter);
  Nan::SetAccessor(proto, JS_STR("kind"), KindGetter);
  Nan::SetAccessor(proto, JS_STR("label"), LabelGetter);
  Nan::SetAccessor(proto, JS_STR("muted"), MutedGetter, MutedSetter);
  Nan::SetAccessor(proto, JS_STR("readyState"), ReadyStateGetter);
  Nan::SetMethod(proto, "stop", Stop);
}

NAN_METHOD(MediaStreamTrack::New) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("MicrophoneMediaStream"))) {
    Local<Object> microphoneMediaStreamObj = Local<Object>::Cast(info[0]);
    MicrophoneMediaStream *microphoneMediaStream = ObjectWrap::Unwrap<MicrophoneMediaStream>(microphoneMediaStreamObj);
    
    MediaStreamTrack *mediaStreamTrack = new MediaStreamTrack(microphoneMediaStream);
    Local<Object> mediaStreamTrackObj = info.This();
    mediaStreamTrack->Wrap(mediaStreamTrackObj);

    info.GetReturnValue().Set(mediaStreamTrackObj);
  } else {
    Nan::ThrowError("MediaStreamTrack: invalid arguments");
  }
}

NAN_GETTER(MediaStreamTrack::EnabledGetter) {
  Nan::HandleScope scope;
  
  info.GetReturnValue().Set(JS_BOOL(true));
}

NAN_GETTER(MediaStreamTrack::KindGetter) {
  Nan::HandleScope scope;
  
  info.GetReturnValue().Set(JS_STR("audio"));
}

NAN_GETTER(MediaStreamTrack::LabelGetter) {
  Nan::HandleScope scope;
  
  info.GetReturnValue().Set(JS_STR("microphone"));
}

NAN_GETTER(MediaStreamTrack::MutedGetter) {
  Nan::HandleScope scope;
  
  MediaStreamTrack *mediaStreamTrack = ObjectWrap::Unwrap<MediaStreamTrack>(info.This());
  info.GetReturnValue().Set(JS_BOOL(mediaStreamTrack->muted));
}

NAN_SETTER(MediaStreamTrack::MutedSetter) {
  Nan::HandleScope scope;
  
  if (value->IsBoolean()) {
    MediaStreamTrack *mediaStreamTrack = ObjectWrap::Unwrap<MediaStreamTrack>(info.This());
    
    bool muted = value->BooleanValue();
    mediaStreamTrack->muted = muted;
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_GETTER(MediaStreamTrack::ReadyStateGetter) {
  Nan::HandleScope scope;
  
  MediaStreamTrack *mediaStreamTrack = ObjectWrap::Unwrap<MediaStreamTrack>(info.This());
  
  info.GetReturnValue().Set(JS_STR(mediaStreamTrack->live ? "live" : "ended"));
}

NAN_METHOD(MediaStreamTrack::Stop) {
  Nan::HandleScope scope;

  MediaStreamTrack *mediaStreamTrack = ObjectWrap::Unwrap<MediaStreamTrack>(info.This());
  MediaStream *mediaStream = mediaStreamTrack->mediaStream;
  mediaStream->audioNode.reset();
}

}

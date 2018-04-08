#include <AudioContext.h>

namespace webaudio {

unique_ptr<lab::AudioContext> _defaultAudioContext = nullptr;

lab::AudioContext *getDefaultAudioContext() {
  if (!_defaultAudioContext) {
    _defaultAudioContext = lab::MakeRealtimeAudioContext();

    atexit([]() {
      _defaultAudioContext.reset();
    });
  }
  return _defaultAudioContext.get();
}

AudioContext::AudioContext() {
  audioContext = getDefaultAudioContext();
}

AudioContext::~AudioContext() {}

Handle<Object> AudioContext::Initialize(Isolate *isolate, Local<Value> audioListenerCons, Local<Value> audioSourceNodeCons, Local<Value> audioDestinationNodeCons, Local<Value> gainNodeCons, Local<Value> analyserNodeCons, Local<Value> pannerNodeCons, Local<Value> stereoPannerNodeCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioContext"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "close", Close);
  Nan::SetMethod(proto, "createMediaElementSource", CreateMediaElementSource);
  Nan::SetMethod(proto, "createMediaStreamSource", CreateMediaStreamSource);
  Nan::SetMethod(proto, "createMediaStreamDestination", CreateMediaStreamDestination);
  Nan::SetMethod(proto, "createMediaStreamTrackSource", CreateMediaStreamTrackSource);
  Nan::SetMethod(proto, "createGain", CreateGain);
  Nan::SetMethod(proto, "createAnalyser", CreateAnalyser);
  Nan::SetMethod(proto, "createPanner", CreatePanner);
  Nan::SetMethod(proto, "createStereoPanner", CreateStereoPanner);
  Nan::SetAccessor(proto, JS_STR("currentTime"), CurrentTimeGetter);
  Nan::SetAccessor(proto, JS_STR("sampleRate"), SampleRateGetter);

  Local<Function> ctorFn = ctor->GetFunction();

  ctorFn->Set(JS_STR("AudioListener"), audioListenerCons);
  ctorFn->Set(JS_STR("AudioSourceNode"), audioSourceNodeCons);
  ctorFn->Set(JS_STR("AudioDestinationNode"), audioDestinationNodeCons);
  ctorFn->Set(JS_STR("GainNode"), gainNodeCons);
  ctorFn->Set(JS_STR("AnalyserNode"), analyserNodeCons);
  ctorFn->Set(JS_STR("PannerNode"), pannerNodeCons);
  ctorFn->Set(JS_STR("StereoPannerNode"), stereoPannerNodeCons);

  return scope.Escape(ctorFn);
}

void AudioContext::Close() {
  Nan::ThrowError("AudioContext::Close: not implemented"); // TODO
}

Local<Object> AudioContext::CreateMediaElementSource(Local<Function> audioDestinationNodeConstructor, Local<Object> mediaElement, Local<Object> audioContextObj) {
  Local<Value> argv[] = {
    mediaElement,
    audioContextObj,
  };
  Local<Object> audioSourceNodeObj = audioDestinationNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return audioSourceNodeObj;
}

Local<Object> AudioContext::CreateMediaStreamSource(Local<Function> audioSourceNodeConstructor, Local<Object> mediaStream, Local<Object> audioContextObj) {
  Local<Value> argv[] = {
    mediaStream,
    audioContextObj,
  };
  Local<Object> audioSourceNodeObj = audioSourceNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return audioSourceNodeObj;
}

void AudioContext::CreateMediaStreamDestination() {
  Nan::ThrowError("AudioContext::CreateMediaStreamDestination: not implemented"); // TODO
}

void AudioContext::CreateMediaStreamTrackSource() {
  Nan::ThrowError("AudioContext::CreateMediaStreamTrackSource: not implemented"); // TODO
}

Local<Object> AudioContext::CreateGain(Local<Function> gainNodeConstructor, Local<Object> audioContextObj) {
  Local<Value> argv[] = {
    audioContextObj,
  };
  Local<Object> gainNodeObj = gainNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return gainNodeObj;
}

Local<Object> AudioContext::CreateAnalyser(Local<Function> analyserNodeConstructor, Local<Object> audioContextObj) {
  Local<Value> argv[] = {
    audioContextObj,
  };
  Local<Object> analyserNodeObj = analyserNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return analyserNodeObj;
}

Local<Object> AudioContext::CreatePanner(Local<Function> pannerNodeConstructor, Local<Object> audioContextObj) {
  Local<Value> argv[] = {
    audioContextObj,
  };
  Local<Object> pannerNodeObj = pannerNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return pannerNodeObj;
}

Local<Object> AudioContext::CreateStereoPanner(Local<Function> stereoPannerNodeConstructor, Local<Object> audioContextObj) {
  Local<Value> argv[] = {
    audioContextObj,
  };
  Local<Object> stereoPannerNodeObj = stereoPannerNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return stereoPannerNodeObj;
}

void AudioContext::Suspend() {
  Nan::HandleScope scope;

  Nan::ThrowError("AudioContext::Suspend: not implemented"); // TODO
}

void AudioContext::Resume() {
  Nan::HandleScope scope;

  Nan::ThrowError("AudioContext::Resume: not implemented"); // TODO
}

NAN_METHOD(AudioContext::New) {
  Nan::HandleScope scope;

  Local<Object> audioContextObj = info.This();
  AudioContext *audioContext = new AudioContext();
  audioContext->Wrap(audioContextObj);

  Local<Function> audioDestinationNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioDestinationNode")));
  Local<Value> argv1[] = {
    audioContextObj,
  };
  Local<Object> audioDestinationNodeObj = audioDestinationNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv1)/sizeof(argv1[0]), argv1).ToLocalChecked();
  audioContextObj->Set(JS_STR("destination"), audioDestinationNodeObj);

  Local<Function> audioListenerConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioListener")));
  Local<Value> argv2[] = {
    audioContextObj,
  };
  Local<Object> audioListenerObj = audioListenerConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv2)/sizeof(argv2[0]), argv2).ToLocalChecked();
  audioContextObj->Set(JS_STR("listener"), audioListenerObj);

  info.GetReturnValue().Set(audioContextObj);
}

NAN_METHOD(AudioContext::Close) {
  Nan::HandleScope scope;

  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(info.This());
  audioContext->Close();
}

NAN_METHOD(AudioContext::CreateMediaElementSource) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("HTMLAudioElement"))) {
    Local<Object> htmlAudioElement = Local<Object>::Cast(info[0]);

    Local<Object> audioContextObj = info.This();
    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

    Local<Function> audioDestinationNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioSourceNode")));
    Local<Object> audioNodeObj = audioContext->CreateMediaElementSource(audioDestinationNodeConstructor, htmlAudioElement, audioContextObj);

    info.GetReturnValue().Set(audioNodeObj);
  } else {
    Nan::ThrowError("AudioContext::CreateMediaElementSource: invalid arguments");
  }
}

NAN_METHOD(AudioContext::CreateMediaStreamSource) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("MicrophoneMediaStream"))) {
    Local<Object> microphoneMediaStream = Local<Object>::Cast(info[0]);

    Local<Object> audioContextObj = info.This();
    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

    Local<Function> audioSourceNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioSourceNode")));
    Local<Object> audioNodeObj = audioContext->CreateMediaStreamSource(audioSourceNodeConstructor, microphoneMediaStream, audioContextObj);

    info.GetReturnValue().Set(audioNodeObj);
  } else {
    Nan::ThrowError("AudioContext::CreateMediaElementSource: invalid arguments");
  }
}

NAN_METHOD(AudioContext::CreateMediaStreamDestination) {
  Nan::HandleScope scope;

  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(info.This());
  audioContext->CreateMediaStreamDestination();
}

NAN_METHOD(AudioContext::CreateMediaStreamTrackSource) {
  Nan::HandleScope scope;

  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(info.This());
  audioContext->CreateMediaStreamTrackSource();
}

NAN_METHOD(AudioContext::CreateGain) {
  Nan::HandleScope scope;

  Local<Object> audioContextObj = info.This();
  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

  Local<Function> gainNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("GainNode")));
  Local<Object> gainNodeObj = audioContext->CreateGain(gainNodeConstructor, audioContextObj);

  info.GetReturnValue().Set(gainNodeObj);
}

NAN_METHOD(AudioContext::CreateAnalyser) {
  Nan::HandleScope scope;

  Local<Object> audioContextObj = info.This();
  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

  Local<Function> analyserNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AnalyserNode")));
  Local<Object> analyserNodeObj = audioContext->CreateAnalyser(analyserNodeConstructor, audioContextObj);

  info.GetReturnValue().Set(analyserNodeObj);
}

NAN_METHOD(AudioContext::CreatePanner) {
  Nan::HandleScope scope;

  Local<Object> audioContextObj = info.This();
  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

  Local<Function> pannerNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("PannerNode")));
  Local<Object> pannerNodeObj = audioContext->CreatePanner(pannerNodeConstructor, audioContextObj);

  info.GetReturnValue().Set(pannerNodeObj);
}

NAN_METHOD(AudioContext::CreateStereoPanner) {
  Nan::HandleScope scope;

  Local<Object> audioContextObj = info.This();
  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

  Local<Function> stereoPannerNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("StereoPannerNode")));
  Local<Object> stereoPannerNodeObj = audioContext->CreateStereoPanner(stereoPannerNodeConstructor, audioContextObj);

  info.GetReturnValue().Set(stereoPannerNodeObj);
}

NAN_METHOD(AudioContext::Suspend) {
  Nan::HandleScope scope;

  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(info.This());
  audioContext->Suspend();
}

NAN_METHOD(AudioContext::Resume) {
  Nan::HandleScope scope;

  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(info.This());
  audioContext->Resume();
}

NAN_GETTER(AudioContext::CurrentTimeGetter) {
  Nan::HandleScope scope;

  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(info.This());
  info.GetReturnValue().Set(JS_NUM(audioContext->audioContext->currentTime()));
}

NAN_GETTER(AudioContext::SampleRateGetter) {
  Nan::HandleScope scope;

  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(info.This());
  info.GetReturnValue().Set(JS_NUM(audioContext->audioContext->sampleRate()));
}

}

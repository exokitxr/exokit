#include <AudioContext.h>

namespace webaudio {

unique_ptr<lab::AudioContext> _defaultAudioContext = nullptr;

lab::AudioContext *getDefaultAudioContext(float sampleRate) {
  if (!_defaultAudioContext) {
    _defaultAudioContext = lab::MakeRealtimeAudioContext(sampleRate);

    atexit([]() {
      _defaultAudioContext.reset();
    });
  }
  return _defaultAudioContext.get();
}

AudioContext::AudioContext(float sampleRate) {
  audioContext = getDefaultAudioContext(sampleRate);
}

AudioContext::~AudioContext() {}

Handle<Object> AudioContext::Initialize(Isolate *isolate, Local<Value> audioListenerCons, Local<Value> audioSourceNodeCons, Local<Value> audioDestinationNodeCons, Local<Value> gainNodeCons, Local<Value> analyserNodeCons, Local<Value> pannerNodeCons, Local<Value> audioBufferCons, Local<Value> audioBufferSourceNodeCons, Local<Value> audioProcessingEventCons, Local<Value> stereoPannerNodeCons, Local<Value> oscillatorNodeCons, Local<Value> scriptProcessorNodeCons, Local<Value> mediaStreamTrackCons, Local<Value> microphoneMediaStreamCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioContext"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "close", Close);
  Nan::SetMethod(proto, "_decodeAudioDataSync", _DecodeAudioDataSync);
  Nan::SetMethod(proto, "createMediaElementSource", CreateMediaElementSource);
  Nan::SetMethod(proto, "createMediaStreamSource", CreateMediaStreamSource);
  Nan::SetMethod(proto, "createMediaStreamDestination", CreateMediaStreamDestination);
  Nan::SetMethod(proto, "createMediaStreamTrackSource", CreateMediaStreamTrackSource);
  Nan::SetMethod(proto, "createGain", CreateGain);
  Nan::SetMethod(proto, "createAnalyser", CreateAnalyser);
  Nan::SetMethod(proto, "createPanner", CreatePanner);
  Nan::SetMethod(proto, "createStereoPanner", CreateStereoPanner);
  Nan::SetMethod(proto, "createOscillator", CreateOscillator);
  Nan::SetMethod(proto, "createBuffer", CreateBuffer);
  Nan::SetMethod(proto, "createBufferSource", CreateBufferSource);
  Nan::SetMethod(proto, "createScriptProcessor", CreateScriptProcessor);
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
  ctorFn->Set(JS_STR("OscillatorNode"), oscillatorNodeCons);
  ctorFn->Set(JS_STR("AudioBuffer"), audioBufferCons);
  ctorFn->Set(JS_STR("AudioBufferSourceNode"), audioBufferSourceNodeCons);
  ctorFn->Set(JS_STR("AudioProcessingEvent"), audioProcessingEventCons);
  ctorFn->Set(JS_STR("ScriptProcessorNode"), scriptProcessorNodeCons);
  ctorFn->Set(JS_STR("MediaStreamTrack"), mediaStreamTrackCons);
  ctorFn->Set(JS_STR("MicrophoneMediaStream"), microphoneMediaStreamCons);

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

Local<Object> AudioContext::CreateOscillator(Local<Function> oscillatorNodeConstructor, Local<Object> audioContextObj) {
  Local<Value> argv[] = {
    audioContextObj,
  };
  Local<Object> oscillatorNodeObj = oscillatorNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return oscillatorNodeObj;
}

Local<Value> AudioContext::_DecodeAudioDataSync(Local<Function> audioBufferConstructor, Local<ArrayBuffer> srcArrayBuffer) {
  size_t bufferLength = srcArrayBuffer->ByteLength();
  vector<uint8_t> buffer(bufferLength);
  memcpy(buffer.data(), srcArrayBuffer->GetContents().Data(), bufferLength);

  string error;
  shared_ptr<lab::AudioBus> audioBus(lab::MakeBusFromMemory(buffer, false, &error));
  if (audioBus) {
    size_t numChannels = audioBus->numberOfChannels();
    Local<Array> sourcesArray = Nan::New<Array>(numChannels);
    size_t numFrames = audioBus->channel(0)->length();

    for (size_t i = 0; i < numChannels; i++) {
      lab::AudioChannel *audioChannel = audioBus->channel(i);
      const float *source = audioChannel->data();

      Local<ArrayBuffer> sourceArrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), numFrames * sizeof(float));
      memcpy(sourceArrayBuffer->GetContents().Data(), source, sourceArrayBuffer->ByteLength());
      Local<Float32Array> sourceFloat32Array = Float32Array::New(sourceArrayBuffer, 0, numFrames);
      sourcesArray->Set(i, sourceFloat32Array);
    }
    Local<Value> argv[] = {
      JS_INT((uint32_t)numChannels),
      JS_INT((uint32_t)numFrames),
      JS_INT((uint32_t)audioContext->sampleRate()),
      sourcesArray,
    };
    Local<Object> audioBuffer = audioBufferConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

    return audioBuffer;
  } else {
    Nan::ThrowError(error.c_str());

    return Nan::Null();
  }
}

Local<Object> AudioContext::CreateBuffer(Local<Function> audioBufferConstructor, uint32_t numOfChannels, uint32_t length, uint32_t sampleRate) {
  Local<Value> argv[] = {
    JS_INT(numOfChannels),
    JS_INT(length),
    JS_INT(sampleRate),
  };
  Local<Object> audioBufferObj = audioBufferConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return audioBufferObj;
}

Local<Object> AudioContext::CreateBufferSource(Local<Function> audioBufferSourceNodeConstructor, Local<Object> audioContextObj) {
  Local<Value> argv[] = {
    audioContextObj,
  };
  Local<Object> audioBufferObj = audioBufferSourceNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return audioBufferObj;
}

Local<Object> AudioContext::CreateScriptProcessor(Local<Function> scriptProcessorNodeConstructor, uint32_t bufferSize, uint32_t numberOfInputChannels, uint32_t numberOfOutputChannels, Local<Object> audioContextObj) {
  Local<Value> argv[] = {
    JS_INT(bufferSize),
    JS_INT(numberOfInputChannels),
    JS_INT(numberOfOutputChannels),
    audioContextObj,
  };
  Local<Object> scriptProcessorNodeObj = scriptProcessorNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

  return scriptProcessorNodeObj;
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
  if (!threadInitialized) {
    uv_async_init(uv_default_loop(), &threadAsync, RunInMainThread);

    atexit([]{
      uv_close((uv_handle_t *)&threadAsync, nullptr);
    });

    threadInitialized = true;
  }

  Local<Object> options = info[0]->IsObject() ? Local<Object>::Cast(info[0]) : Nan::New<Object>();
  Local<Value> sampleRateValue = options->Get(JS_STR("sampleRate"));
  float sampleRate = sampleRateValue->IsNumber() ? sampleRateValue->NumberValue() : lab::DefaultSampleRate;

  Local<Object> audioContextObj = info.This();
  AudioContext *audioContext = new AudioContext(sampleRate);
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

NAN_METHOD(AudioContext::_DecodeAudioDataSync) {
  Nan::HandleScope scope;

  if (info[0]->IsArrayBuffer()) {
    Local<Object> audioContextObj = info.This();
    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

    Local<ArrayBuffer> srcArrayBuffer = Local<ArrayBuffer>::Cast(info[0]);

    Local<Function> audioBufferConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioBuffer")));
    Local<Value> audioBufferObj = audioContext->_DecodeAudioDataSync(audioBufferConstructor, srcArrayBuffer);

    info.GetReturnValue().Set(audioBufferObj);
  } else {
    Nan::ThrowError("AudioContext::_DecodeAudioDataSync: invalid arguments");
  }
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

NAN_METHOD(AudioContext::CreateOscillator) {
  Nan::HandleScope scope;

  Local<Object> audioContextObj = info.This();
  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

  Local<Function> oscillatorNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("OscillatorNode")));
  Local<Object> oscillatorNodeObj = audioContext->CreateOscillator(oscillatorNodeConstructor, audioContextObj);

  info.GetReturnValue().Set(oscillatorNodeObj);
}

NAN_METHOD(AudioContext::CreateBuffer) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    uint32_t numOfChannels = info[0]->Uint32Value();
    uint32_t length = info[1]->Uint32Value();
    uint32_t sampleRate = info[2]->Uint32Value();

    Local<Object> audioContextObj = info.This();
    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

    Local<Function> audioBufferConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioBuffer")));
    Local<Object> audioBufferObj = audioContext->CreateBuffer(audioBufferConstructor, numOfChannels, length, sampleRate);

    info.GetReturnValue().Set(audioBufferObj);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(AudioContext::CreateBufferSource) {
  Nan::HandleScope scope;

  Local<Object> audioContextObj = info.This();
  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

  Local<Function> audioBufferSourceNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioBufferSourceNode")));
  Local<Object> audioBufferSourceNodeObj = audioContext->CreateBufferSource(audioBufferSourceNodeConstructor, audioContextObj);

  info.GetReturnValue().Set(audioBufferSourceNodeObj);
}

NAN_METHOD(AudioContext::CreateScriptProcessor) {
  Nan::HandleScope scope;

  uint32_t bufferSize = info[0]->IsNumber() ? info[0]->Uint32Value() : 256;
  uint32_t numberOfInputChannels = info[1]->IsNumber() ? info[1]->Uint32Value() : 2;
  uint32_t numberOfOutputChannels = info[2]->IsNumber() ? info[2]->Uint32Value() : 2;

  Local<Object> audioContextObj = info.This();
  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

  Local<Function> scriptProcessorNodeConstructor = Local<Function>::Cast(audioContextObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("ScriptProcessorNode")));
  Local<Object> scriptProcessorNodeObj = audioContext->CreateScriptProcessor(scriptProcessorNodeConstructor, bufferSize, numberOfInputChannels, numberOfOutputChannels, audioContextObj);

  info.GetReturnValue().Set(scriptProcessorNodeObj);
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

#include "LabSound/core/ConcurrentQueue.h"
struct msg_t {
    function<void()> threadFn;
};
lab::concurrent_queue<msg_t> queue;
uv_async_t threadAsync;
bool threadInitialized = false;
void QueueOnMainThread(lab::ContextRenderLock &r, function<void()> &&newThreadFn) {
  msg_t msg;
  msg.threadFn = std::move(newThreadFn);
  queue.push(msg);
  uv_async_send(&threadAsync);
}
void RunInMainThread(uv_async_t *handle) {
  msg_t msg;
  while (queue.try_pop(msg)) {
      msg.threadFn();
  }
}

}

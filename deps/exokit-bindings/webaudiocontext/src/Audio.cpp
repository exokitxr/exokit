#include <Audio.h>
#include <memory>
#include <algorithm>

namespace webaudio {

Audio::Audio() {
  WebAudioAsync *webaudioAsync = getWebAudioAsync();
  audioNode.reset(new lab::FinishableSourceNode(
    [this, webaudioAsync](lab::ContextRenderLock &r){
      webaudioAsync->QueueOnMainThread(r, std::bind(ProcessInMainThread, this));
    }
  ));
}

Audio::~Audio() {}

Local<Object> Audio::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Audio"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "load", Load);
  Nan::SetMethod(proto, "play", Play);
  Nan::SetMethod(proto, "pause", Pause);
  Nan::SetAccessor(proto, JS_STR("paused"), PausedGetter);
  Nan::SetAccessor(proto, JS_STR("currentTime"), CurrentTimeGetter, CurrentTimeSetter);
  Nan::SetAccessor(proto, JS_STR("duration"), DurationGetter);
  Nan::SetAccessor(proto, JS_STR("loop"), LoopGetter, LoopSetter);
  Nan::SetAccessor(proto, JS_STR("onended"), OnEndedGetter, OnEndedSetter);

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();

  return scope.Escape(ctorFn);
}

NAN_METHOD(Audio::New) {
  // Nan::HandleScope scope;

  Audio *audio = new Audio();
  Local<Object> audioObj = info.This();
  audio->Wrap(audioObj);

  info.GetReturnValue().Set(audioObj);
}

void Audio::Load(uint8_t *bufferValue, size_t bufferLength) {
  vector<uint8_t> buffer(bufferLength);
  memcpy(buffer.data(), bufferValue, bufferLength);

  string error;
  shared_ptr<lab::AudioBus> audioBus(lab::MakeBusFromMemory(buffer, false, &error));
  if (audioBus) {
    lab::AudioContext *defaultAudioContext = getDefaultAudioContext();
    {
      lab::ContextRenderLock lock(defaultAudioContext, "Audio::Load");
      audioNode->setBus(lock, audioBus);
    }

    defaultAudioContext->connect(defaultAudioContext->destination(), audioNode, 0, 0); // default connection
  } else {
    Nan::ThrowError(error.c_str());
  }
}

void Audio::Play() {
  audioNode->start(0);
}

void Audio::Pause() {
  audioNode->stop(0);
}

NAN_METHOD(Audio::Load) {
  if (info[0]->IsArrayBuffer()) {
    Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

    Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(info[0]);

    audio->Load((uint8_t *)arrayBuffer->GetContents().Data(), arrayBuffer->ByteLength());
  } else if (info[0]->IsTypedArray()) {
    Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

    Local<ArrayBufferView> arrayBufferView = Local<ArrayBufferView>::Cast(info[0]);
    Local<ArrayBuffer> arrayBuffer = arrayBufferView->Buffer();

    audio->Load((uint8_t *)arrayBuffer->GetContents().Data() + arrayBufferView->ByteOffset(), arrayBufferView->ByteLength());
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(Audio::Play) {
  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());
  audio->Play();
}

NAN_METHOD(Audio::Pause) {
  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());
  audio->Pause();
}

NAN_GETTER(Audio::PausedGetter) {
  // Nan::HandleScope scope;

  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());
  bool paused = !audio->audioNode->isPlayingOrScheduled();

  info.GetReturnValue().Set(JS_BOOL(paused));
}

NAN_GETTER(Audio::CurrentTimeGetter) {
  // Nan::HandleScope scope;

  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

  double now = getDefaultAudioContext()->currentTime();
  double startTime = audio->audioNode->startTime();
  double duration = audio->audioNode->duration();
  double currentTime = std::min<double>(std::max<double>(startTime - now, 0), duration);

  info.GetReturnValue().Set(JS_NUM(currentTime));
}

NAN_SETTER(Audio::CurrentTimeSetter) {
  // Nan::HandleScope scope;

  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

  if (value->IsNumber()) {
    double currentTime = TO_DOUBLE(value);

    audio->audioNode->setCurrentTime(currentTime);
  } else {
    Nan::ThrowError("loop: invalid arguments");
  }
}

NAN_GETTER(Audio::DurationGetter) {
  // Nan::HandleScope scope;

  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

  double duration = audio->audioNode->duration();
  info.GetReturnValue().Set(JS_NUM(duration));
}

NAN_GETTER(Audio::LoopGetter) {
  // Nan::HandleScope scope;

  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

  bool loop = audio->audioNode->loop();
  info.GetReturnValue().Set(JS_BOOL(loop));
}

NAN_SETTER(Audio::LoopSetter) {
  // Nan::HandleScope scope;

  if (value->IsBoolean()) {
    bool loop = TO_BOOL(value);

    Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

    audio->audioNode->setLoop(loop);
  } else {
    Nan::ThrowError("loop: invalid arguments");
  }
}

NAN_GETTER(Audio::OnEndedGetter) {
  // Nan::HandleScope scope;

  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());
  Local<Function> onended = Nan::New(audio->onended);
  info.GetReturnValue().Set(onended);
}
NAN_SETTER(Audio::OnEndedSetter) {
  // Nan::HandleScope scope;

  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

  if (value->IsFunction()) {
    Local<Function> onended = Local<Function>::Cast(value);
    audio->onended.Reset(onended);
  } else {
    audio->onended.Reset();
  }
}
void Audio::ProcessInMainThread(Audio *self) {
  Nan::HandleScope scope;

  if (!self->onended.IsEmpty()) {
    Local<Function> onended = Nan::New(self->onended);
    onended->Call(Isolate::GetCurrent()->GetCurrentContext(), Nan::Null(), 0, nullptr);
  }
}

}

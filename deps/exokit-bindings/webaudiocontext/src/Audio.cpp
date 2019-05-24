#include <Audio.h>
#include <memory>
#include <algorithm>

namespace webaudio {

Audio::Audio() : loaded(false), connected(false), audioContext(nullptr) {
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

void Audio::Load(uint8_t *bufferValue, size_t bufferLength, Local<Function> cbFn) {
  if (this->cbFn.IsEmpty()) {
    this->cbFn.Reset(cbFn);

    WebAudioAsync *webAudioAsync = getWebAudioAsync();
    std::vector<unsigned char> buffer(bufferLength);
    memcpy(buffer.data(), bufferValue, bufferLength);
    std::thread([this, webAudioAsync, buffer{std::move(buffer)}]() mutable -> void {
      this->audioBus = lab::MakeBusFromMemory(buffer, false, &this->error);

      webAudioAsync->QueueOnMainThread(std::bind(ProcessLoadInMainThread, this));
    }).detach();
  } else {
    Local<String> arg0 = Nan::New<String>("already loading").ToLocalChecked();
    Local<Value> argv[] = {
      arg0,
    };
    cbFn->Call(Isolate::GetCurrent()->GetCurrentContext(), Nan::Null(), sizeof(argv)/sizeof(argv[0]), argv);
  }
}

void Audio::ProcessLoadInMainThread(Audio *audio) {
  Nan::HandleScope scope;

  lab::AudioContext *localAudioContext = audio->audioContext;
  if (!localAudioContext) {
    localAudioContext = getDefaultAudioContext();
  }
  {
    lab::ContextRenderLock lock(localAudioContext, "Audio::ProcessLoadInMainThread");

    audio->audioNode->setBus(lock, audio->audioBus);

    /* if (!audio->audioContext) {
      audio->audioContext = getDefaultAudioContext();
      audio->audioContext->connect(audio->audioContext->destination(), audio->audioNode, 0, 0);
    } */
  }

  Local<Object> asyncObject = Nan::New<Object>();
  AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "Audio::ProcessLoadInMainThread");

  Local<Function> cbFn = Nan::New(audio->cbFn);
  Local<String> arg0 = Nan::New<String>(audio->error).ToLocalChecked();
  Local<Value> argv[] = {
    arg0,
  };
  asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);

  audio->cbFn.Reset();
  audio->audioBus.reset();
  audio->error = "";
}

void Audio::Play() {
  audioNode->start(0);
}

void Audio::Pause() {
  audioNode->stop(0);
}

// for when we reparent to MediaElementSourceNode
void Audio::Reparent(AudioContext *newAudioContext) {
  if (audioContext) {
    audioContext->disconnect(nullptr, audioNode);
    lab::ContextRenderLock lock(audioContext, "Audio::Reparent"); // commit the change
  }
  
  audioContext = newAudioContext->audioContext.get();
}

NAN_METHOD(Audio::Load) {

  if (info[1]->IsFunction()) {
    if (info[0]->IsArrayBuffer()) {
      Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

      Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(info[0]);
      Local<Function> cbFn = Local<Function>::Cast(info[1]);

      audio->Load((uint8_t *)arrayBuffer->GetContents().Data(), arrayBuffer->ByteLength(), cbFn);
    } else if (info[0]->IsTypedArray()) {
      Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

      Local<ArrayBufferView> arrayBufferView = Local<ArrayBufferView>::Cast(info[0]);
      Local<Function> cbFn = Local<Function>::Cast(info[1]);

      audio->Load((uint8_t *)arrayBufferView->Buffer()->GetContents().Data() + arrayBufferView->ByteOffset(), arrayBufferView->ByteLength(), cbFn);
    } else {
      Nan::ThrowError("invalid arguments");
    }
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

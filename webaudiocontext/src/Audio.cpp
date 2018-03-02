#include <Audio.h>
#include <memory>
#include <algorithm>

namespace webaudio {

Audio::Audio() {}
Audio::~Audio() {}

Handle<Object> Audio::Initialize(Isolate *isolate) {
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
  Nan::SetAccessor(proto, JS_STR("currentTime"), CurrentTimeGetter);
  Nan::SetAccessor(proto, JS_STR("duration"), DurationGetter);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_METHOD(Audio::New) {
  Nan::HandleScope scope;

  Audio *audio = new Audio();
  Local<Object> audioObj = info.This();
  audio->Wrap(audioObj);

  info.GetReturnValue().Set(audioObj);
}

const char *getAudioType(const vector<uint8_t> &buf) {
	if (
    buf.size() >= 4 &&
    buf[0] == 102 &&
    buf[1] == 76 &&
    buf[2] == 97 &&
    buf[3] == 67
  ) {
    return "flac";
  } else if (
    buf.size() >= 3 &&
    (
      (
        buf[0] == 73 &&
        buf[1] == 68 &&
        buf[2] == 51
      ) || (
        buf[0] == 255 &&
        buf[1] == 251
      )
    )
  ) {
    return "mp3";
  } else if (
    buf.size() >= 4 &&
    buf[0] == 79 &&
		buf[1] == 103 &&
		buf[2] == 103 &&
    buf[3] == 83
  ) {
    return "ogg";
  } else if (
    buf.size() >= 12 &&
    buf[0] == 82 &&
		buf[1] == 73 &&
		buf[2] == 70 &&
    buf[3] == 70 &&
    buf[8] == 87 &&
    buf[9] == 65 &&
    buf[10] == 86 &&
    buf[11] == 69
  ) {
    return "wav";
  } else {
    return nullptr;
  }
}

void Audio::Load(uint8_t *bufferValue, size_t bufferLength) {
  vector<uint8_t> buffer(bufferLength);
  memcpy(buffer.data(), bufferValue, bufferLength);

  const char *extensionString = getAudioType(buffer);
  if (extensionString != nullptr) {
    string extension(extensionString);

    audioBus = lab::MakeBusFromMemory(buffer, extension, false);
    audioNode.reset(new lab::SampledAudioNode());

    {
      lab::ContextRenderLock lock(defaultAudioContext.get(), "Audio::Load");
      audioNode->setBus(lock, audioBus);
    }

    defaultAudioContext->connect(defaultAudioContext->destination(), audioNode, 0, 0); // default connection
  } else {
    Nan::ThrowError("could not detect audio format");
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

NAN_GETTER(Audio::CurrentTimeGetter) {
  Nan::HandleScope scope;

  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

  double now = defaultAudioContext->currentTime();
  double startTime = audio->audioNode->startTime();
  double duration = audio->audioNode->duration();
  double currentTime = std::min<double>(std::max<double>(startTime - now, 0), duration);

  info.GetReturnValue().Set(JS_NUM(currentTime));
}

NAN_GETTER(Audio::DurationGetter) {
  Nan::HandleScope scope;

  Audio *audio = ObjectWrap::Unwrap<Audio>(info.This());

  double duration = audio->audioNode->duration();

  info.GetReturnValue().Set(JS_NUM(duration));
}

}

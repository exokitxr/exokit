#include <AudioBuffer.h>
#include <AudioContext.h>

namespace webaudio {

AudioBuffer::AudioBuffer(uint32_t sampleRate, Local<Array> buffers) : sampleRate(sampleRate), buffers(buffers) {}
AudioBuffer::~AudioBuffer() {}
Handle<Object> AudioBuffer::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioBuffer"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetAccessor(proto, JS_STR("sampleRate"), SampleRate);
  Nan::SetAccessor(proto, JS_STR("length"), Length);
  Nan::SetAccessor(proto, JS_STR("duration"), Duration);
  Nan::SetAccessor(proto, JS_STR("numberOfChannels"), NumberOfChannels);
  Nan::SetMethod(proto, "getChannelData", GetChannelData);
  Nan::SetMethod(proto, "copyFromChannel", CopyFromChannel);
  Nan::SetMethod(proto, "copyToChannel", CopyToChannel);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}
NAN_METHOD(AudioBuffer::New) {
  // Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    uint32_t numOfChannels = info[0]->Uint32Value();
    uint32_t length = info[1]->Uint32Value();
    uint32_t sampleRate = info[2]->Uint32Value();
    Local<Array> buffers;
    if (info[3]->IsArray()) {
      buffers = Local<Array>::Cast(info[3]);
    } else {
      buffers = Nan::New<Array>(numOfChannels);
      for (size_t i = 0; i < numOfChannels; i++) {
        Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), length * sizeof(float));
        Local<Float32Array> float32Array = Float32Array::New(arrayBuffer, 0, length);
        buffers->Set(i, float32Array);
      }
    }

    AudioBuffer *audioBuffer = new AudioBuffer(sampleRate, buffers);
    Local<Object> audioBufferObj = info.This();
    audioBuffer->Wrap(audioBufferObj);
  } else {
    Nan::ThrowError("AudioBuffer:New: invalid arguments");
  }
}
NAN_GETTER(AudioBuffer::SampleRate) {
  // Nan::HandleScope scope;

  AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(info.This());
  info.GetReturnValue().Set(JS_INT(audioBuffer->sampleRate));
}
NAN_GETTER(AudioBuffer::Length) {
  // Nan::HandleScope scope;

  AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(info.This());
  Local<Array> buffers = Nan::New(audioBuffer->buffers);
  if (buffers->Length() > 0) {
    Local<Float32Array> firstBuffer = Local<Float32Array>::Cast(buffers->Get(0));
    uint32_t numFrames = firstBuffer->Length();

    info.GetReturnValue().Set(JS_INT(numFrames));
  } else {
    info.GetReturnValue().Set(JS_INT(0));
  }
}
NAN_GETTER(AudioBuffer::Duration) {
  // Nan::HandleScope scope;

  AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(info.This());
  Local<Array> buffers = Nan::New(audioBuffer->buffers);
  if (buffers->Length() > 0) {
    Local<Float32Array> firstBuffer = Local<Float32Array>::Cast(buffers->Get(0));
    double numFrames = firstBuffer->Length();
    double sampleRate = audioBuffer->sampleRate;
    double duration = numFrames / sampleRate;

    info.GetReturnValue().Set(JS_NUM(duration));
  } else {
    info.GetReturnValue().Set(JS_NUM(0));
  }
}
NAN_GETTER(AudioBuffer::NumberOfChannels) {
  // Nan::HandleScope scope;

  AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(info.This());
  Local<Array> buffers = Nan::New(audioBuffer->buffers);

  info.GetReturnValue().Set(JS_INT(buffers->Length()));
}
NAN_METHOD(AudioBuffer::GetChannelData) {
  // Nan::HandleScope scope;

  if (info[0]->IsNumber()) {
    uint32_t channelIndex = info[0]->Uint32Value();

    AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(info.This());
    Local<Array> buffers = Nan::New(audioBuffer->buffers);
    Local<Value> channelData = buffers->Get(channelIndex);

    info.GetReturnValue().Set(channelData);
  } else {
    Nan::ThrowError("AudioBuffer:GetChannelData: invalid arguments");
  }
}
NAN_METHOD(AudioBuffer::CopyFromChannel) {
  // Nan::HandleScope scope;

  if (info[0]->IsFloat32Array() && info[1]->IsNumber()) {
    Local<Float32Array> destinationFloat32Array = Local<Float32Array>::Cast(info[0]);
    uint32_t channelIndex = info[1]->Uint32Value();

    AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(info.This());
    Local<Array> buffers = Nan::New(audioBuffer->buffers);
    Local<Value> channelData = buffers->Get(channelIndex);

    if (channelData->BooleanValue()) {
      Local<Float32Array> channelDataFloat32Array = Local<Float32Array>::Cast(channelData);
      uint32_t offset = std::min<uint32_t>(info[2]->IsNumber() ? info[2]->Uint32Value() : 0, channelDataFloat32Array->Length());
      size_t copyLength = std::min<size_t>(channelDataFloat32Array->Length() - offset, destinationFloat32Array->Length());
      for (size_t i = 0; i < copyLength; i++) {
        destinationFloat32Array->Set(i, channelDataFloat32Array->Get(offset + i));
      }
    } else {
      Nan::ThrowError("AudioBuffer:CopyFromChannel: invalid channel index");
    }
  } else {
    Nan::ThrowError("AudioBuffer:CopyFromChannel: invalid arguments");
  }
}
NAN_METHOD(AudioBuffer::CopyToChannel) {
  // Nan::HandleScope scope;

  if (info[0]->IsFloat32Array() && info[1]->IsNumber()) {
    Local<Float32Array> sourceFloat32Array = Local<Float32Array>::Cast(info[0]);
    uint32_t channelIndex = info[1]->Uint32Value();

    AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(info.This());
    Local<Array> buffers = Nan::New(audioBuffer->buffers);
    Local<Value> channelData = buffers->Get(channelIndex);

    if (channelData->BooleanValue()) {
      Local<Float32Array> channelDataFloat32Array = Local<Float32Array>::Cast(channelData);
      uint32_t offset = std::min<uint32_t>(info[2]->IsNumber() ? info[2]->Uint32Value() : 0, channelDataFloat32Array->Length());
      size_t copyLength = std::min<size_t>(channelDataFloat32Array->Length() - offset, sourceFloat32Array->Length());
      for (size_t i = 0; i < copyLength; i++) {
        channelDataFloat32Array->Set(offset + i, sourceFloat32Array->Get(i));
      }
    } else {
      Nan::ThrowError("AudioBuffer:CopyToChannel: invalid channel index");
    }
  } else {
    Nan::ThrowError("AudioBuffer:CopyToChannel: invalid arguments");
  }
}

AudioBufferSourceNode::AudioBufferSourceNode() {
  audioNode.reset(new lab::FinishableSourceNode([this](lab::ContextRenderLock &r){
  }));
}
AudioBufferSourceNode::~AudioBufferSourceNode() {}
Handle<Object> AudioBufferSourceNode::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioBufferSourceNode"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  AudioBufferSourceNode::InitializePrototype(proto);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}
void AudioBufferSourceNode::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetAccessor(proto, JS_STR("buffer"), BufferGetter, BufferSetter);
  Nan::SetAccessor(proto, JS_STR("onended"), OnEndedGetter, OnEndedSetter);
  Nan::SetMethod(proto, "start", Start);
  Nan::SetMethod(proto, "stop", Stop);
}
NAN_METHOD(AudioBufferSourceNode::New) {
  // Nan::HandleScope scope;

  if (info[0]->IsObject() && info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[0]);
    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

    AudioBufferSourceNode *audioBufferSourceNode = new AudioBufferSourceNode();
    Local<Object> audioBufferSourceNodeObj = info.This();
    audioBufferSourceNode->Wrap(audioBufferSourceNodeObj);

    audioBufferSourceNode->context.Reset(audioContextObj);

    info.GetReturnValue().Set(audioBufferSourceNodeObj);
  } else {
    Nan::ThrowError("AudioBufferSourceNode:New: invalid arguments");
  }
}
NAN_METHOD(AudioBufferSourceNode::Start) {
  // Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());
  ((lab::FinishableSourceNode *)audioBufferSourceNode->audioNode.get())->start(0);
}
NAN_METHOD(AudioBufferSourceNode::Stop) {
  // Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());
  ((lab::FinishableSourceNode *)audioBufferSourceNode->audioNode.get())->stop(0);
}
NAN_GETTER(AudioBufferSourceNode::BufferGetter) {
  // Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());
  Local<Object> buffer = Nan::New(audioBufferSourceNode->buffer);
  info.GetReturnValue().Set(buffer);
}
NAN_SETTER(AudioBufferSourceNode::BufferSetter) {
  // Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());
  Local<Object> audioContextObj = Nan::New(audioBufferSourceNode->context);
  AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);
  lab::FinishableSourceNode *audioNode = (lab::FinishableSourceNode *)audioBufferSourceNode->audioNode.get();

  if (value->IsObject() && value->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioBuffer"))) {
    Local<Object> audioBufferObj = Local<Object>::Cast(value);
    audioBufferSourceNode->buffer.Reset(audioBufferObj);

    AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(audioBufferObj);
    Local<Array> buffers = Nan::New(audioBuffer->buffers);
    size_t numChannels = buffers->Length();
    size_t numFrames = numChannels > 0 ? Local<Float32Array>::Cast(buffers->Get(0))->Length() : 0;

    unique_ptr<float *[]> frames(new float*[numChannels]);
    for (size_t i = 0; i < numChannels; i++) {
      Local<Float32Array> bufferFramesFloat32Array = Local<Float32Array>::Cast(buffers->Get(i));
      size_t numBufferFrames = bufferFramesFloat32Array->Length();
      Local<ArrayBuffer> bufferFramesArrayBuffer = bufferFramesFloat32Array->Buffer();
      frames[i] = (float *)((unsigned char *)bufferFramesArrayBuffer->GetContents().Data() + bufferFramesFloat32Array->ByteOffset());
    }

    shared_ptr<lab::AudioBus> audioBus(lab::MakeBusFromRawBuffer(audioContext->audioContext->sampleRate(), numChannels, numFrames, frames.get(), false).release());

    {
      lab::ContextRenderLock lock(audioContext->audioContext, "AudioBufferSourceNode::buffer");
      audioNode->reset(lock);
      audioNode->setBus(lock, audioBus);
    }
  } else {
    audioBufferSourceNode->buffer.Reset();

    {
      lab::ContextRenderLock lock(audioContext->audioContext, "AudioBufferSourceNode::buffer");
      audioNode->reset(lock);
      audioNode->setBus(lock, nullptr);
    }
  }
}
NAN_GETTER(AudioBufferSourceNode::OnEndedGetter) {
  // Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());
  Local<Function> onended = Nan::New(audioBufferSourceNode->onended);
  info.GetReturnValue().Set(onended);
}
NAN_SETTER(AudioBufferSourceNode::OnEndedSetter) {
  // Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());

  if (value->IsFunction()) {
    Local<Function> onended = Local<Function>::Cast(value);
    audioBufferSourceNode->onended.Reset(onended);
  } else {
    audioBufferSourceNode->onended.Reset();
  }
}
void AudioBufferSourceNode::ProcessInMainThread(AudioBufferSourceNode *self) {
  Nan::HandleScope scope;

  if (!self->onended.IsEmpty()) {
    Local<Function> onended = Nan::New(self->onended);
    onended->Call(Nan::Null(), 0, nullptr);
  }
}

}

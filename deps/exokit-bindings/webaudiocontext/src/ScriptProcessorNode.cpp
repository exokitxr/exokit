#include <ScriptProcessorNode.h>
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
  Nan::SetAccessor(proto, JS_STR("numberOfChannels"), NumberOfChannels);
  Nan::SetMethod(proto, "getChannelData", GetChannelData);
  Nan::SetMethod(proto, "copyFromChannel", CopyFromChannel);
  Nan::SetMethod(proto, "copyToChannel", CopyToChannel);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}
NAN_METHOD(AudioBuffer::New) {
  Nan::HandleScope scope;

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
  Nan::HandleScope scope;

  AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(info.This());
  info.GetReturnValue().Set(JS_INT(audioBuffer->sampleRate));
}
NAN_GETTER(AudioBuffer::Length) {
  Nan::HandleScope scope;

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
NAN_GETTER(AudioBuffer::NumberOfChannels) {
  Nan::HandleScope scope;

  AudioBuffer *audioBuffer = ObjectWrap::Unwrap<AudioBuffer>(info.This());
  Local<Array> buffers = Nan::New(audioBuffer->buffers);

  info.GetReturnValue().Set(JS_INT(buffers->Length()));
}
NAN_METHOD(AudioBuffer::GetChannelData) {
  Nan::HandleScope scope;

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
  Nan::HandleScope scope;

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
  Nan::HandleScope scope;

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
    QueueOnMainThread(r, std::bind(ProcessInMainThread, this));
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
  Nan::HandleScope scope;

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
  Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());
  ((lab::FinishableSourceNode *)audioBufferSourceNode->audioNode.get())->start(0);
}
NAN_METHOD(AudioBufferSourceNode::Stop) {
  Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());
  ((lab::FinishableSourceNode *)audioBufferSourceNode->audioNode.get())->stop(0);
}
NAN_GETTER(AudioBufferSourceNode::BufferGetter) {
  Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());
  Local<Object> buffer = Nan::New(audioBufferSourceNode->buffer);
  info.GetReturnValue().Set(buffer);
}
NAN_SETTER(AudioBufferSourceNode::BufferSetter) {
  Nan::HandleScope scope;

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
  Nan::HandleScope scope;

  AudioBufferSourceNode *audioBufferSourceNode = ObjectWrap::Unwrap<AudioBufferSourceNode>(info.This());
  Local<Function> onended = Nan::New(audioBufferSourceNode->onended);
  info.GetReturnValue().Set(onended);
}
NAN_SETTER(AudioBufferSourceNode::OnEndedSetter) {
  Nan::HandleScope scope;

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

AudioProcessingEvent::AudioProcessingEvent(Local<Object> inputBuffer, Local<Object> outputBuffer) : inputBuffer(inputBuffer), outputBuffer(outputBuffer) {}
AudioProcessingEvent::~AudioProcessingEvent() {}
Handle<Object> AudioProcessingEvent::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioProcessingEvent"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetAccessor(proto, JS_STR("inputBuffer"), InputBufferGetter);
  Nan::SetAccessor(proto, JS_STR("outputBuffer"), OutputBufferGetter);
  Nan::SetAccessor(proto, JS_STR("numberOfInputChannels"), NumberOfInputChannelsGetter);
  Nan::SetAccessor(proto, JS_STR("numberOfOutputChannels"), NumberOfOutputChannelsGetter);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}
NAN_METHOD(AudioProcessingEvent::New) {
  Nan::HandleScope scope;

  if (info[0]->IsObject() && info[1]->IsObject()) {
    Local<Object> inputBuffer = Local<Object>::Cast(info[0]);
    Local<Object> outputBuffer = Local<Object>::Cast(info[1]);

    AudioProcessingEvent *audioProcessingEvent = new AudioProcessingEvent(inputBuffer, outputBuffer);
    Local<Object> audioProcessingEventObj = info.This();
    audioProcessingEvent->Wrap(audioProcessingEventObj);
  } else {
    Nan::ThrowError("AudioProcessingEvent:New: invalid arguments");
  }
}
NAN_GETTER(AudioProcessingEvent::InputBufferGetter) {
  Nan::HandleScope scope;

  AudioProcessingEvent *audioProcessingEvent = ObjectWrap::Unwrap<AudioProcessingEvent>(info.This());
  Local<Object> inputBufferObj = Nan::New(audioProcessingEvent->inputBuffer);
  info.GetReturnValue().Set(inputBufferObj);
}
NAN_GETTER(AudioProcessingEvent::OutputBufferGetter) {
  Nan::HandleScope scope;

  AudioProcessingEvent *audioProcessingEvent = ObjectWrap::Unwrap<AudioProcessingEvent>(info.This());
  Local<Object> outputBufferObj = Nan::New(audioProcessingEvent->outputBuffer);
  info.GetReturnValue().Set(outputBufferObj);
}
NAN_GETTER(AudioProcessingEvent::NumberOfInputChannelsGetter) {
  Nan::HandleScope scope;

  AudioProcessingEvent *audioProcessingEvent = ObjectWrap::Unwrap<AudioProcessingEvent>(info.This());
  Local<Object> inputBufferObj = Nan::New(audioProcessingEvent->inputBuffer);
  Local<Value> numberOfChannels = inputBufferObj->Get(JS_STR("numberOfChannels"));
  info.GetReturnValue().Set(numberOfChannels);
}
NAN_GETTER(AudioProcessingEvent::NumberOfOutputChannelsGetter) {
  Nan::HandleScope scope;

  AudioProcessingEvent *audioProcessingEvent = ObjectWrap::Unwrap<AudioProcessingEvent>(info.This());
  Local<Object> outpuctBufferObj = Nan::New(audioProcessingEvent->outputBuffer);
  Local<Value> numberOfChannels = outpuctBufferObj->Get(JS_STR("numberOfChannels"));
  info.GetReturnValue().Set(numberOfChannels);
}

ScriptProcessorNode::ScriptProcessorNode() {}
ScriptProcessorNode::~ScriptProcessorNode() {}
Handle<Object> ScriptProcessorNode::Initialize(Isolate *isolate, Local<Value> audioBufferCons, Local<Value> audioProcessingEventCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("ScriptProcessorNode"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  ScriptProcessorNode::InitializePrototype(proto);

  Local<Function> ctorFn = ctor->GetFunction();

  ctorFn->Set(JS_STR("AudioBuffer"), audioBufferCons);
  ctorFn->Set(JS_STR("AudioProcessingEvent"), audioProcessingEventCons);

  return scope.Escape(ctorFn);
}
void ScriptProcessorNode::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetAccessor(proto, JS_STR("onaudioprocess"), OnAudioProcessGetter, OnAudioProcessSetter);
}
NAN_METHOD(ScriptProcessorNode::New) {
  Nan::HandleScope scope;

  if (
    info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() &&
    info[3]->IsObject() && info[3]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))
  ) {
    uint32_t bufferSize = info[0]->Uint32Value();
    uint32_t numberOfInputChannels = info[1]->Uint32Value();
    uint32_t numberOfOutputChannels = info[2]->Uint32Value();

    if (numberOfInputChannels == numberOfOutputChannels) {
      Local<Object> audioContextObj = Local<Object>::Cast(info[3]);

      ScriptProcessorNode *scriptProcessorNode = new ScriptProcessorNode();
      Local<Object> scriptProcessorNodeObj = info.This();
      scriptProcessorNode->Wrap(scriptProcessorNodeObj);

      scriptProcessorNode->context.Reset(audioContextObj);
      lab::ScriptProcessorNode *labScriptProcessorNode = new lab::ScriptProcessorNode(numberOfInputChannels, [scriptProcessorNode](lab::ContextRenderLock& r, vector<const float*> sources, vector<float*> destinations, size_t framesToProcess) {
        scriptProcessorNode->ProcessInAudioThread(r, sources, destinations, framesToProcess);
      });
      scriptProcessorNode->audioNode.reset(labScriptProcessorNode);

      Local<Function> audioBufferConstructor = Local<Function>::Cast(scriptProcessorNodeObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioBuffer")));
      scriptProcessorNode->audioBufferConstructor.Reset(audioBufferConstructor);
      Local<Function> audioProcessingEventConstructor = Local<Function>::Cast(scriptProcessorNodeObj->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("AudioProcessingEvent")));
      scriptProcessorNode->audioProcessingEventConstructor.Reset(audioProcessingEventConstructor);

      info.GetReturnValue().Set(scriptProcessorNodeObj);
    } else {
      Nan::ThrowError("ScriptProcessorNode: numberOfInputChannels and numberOfOutputChannels do not match");
    }
  } else {
    Nan::ThrowError("ScriptProcessorNode: invalid arguments");
  }
}
NAN_GETTER(ScriptProcessorNode::OnAudioProcessGetter) {
  Nan::HandleScope scope;

  ScriptProcessorNode *scriptProcessorNode = ObjectWrap::Unwrap<ScriptProcessorNode>(info.This());

  Local<Function> onAudioProcessLocal = Nan::New(scriptProcessorNode->onAudioProcess);
  info.GetReturnValue().Set(onAudioProcessLocal);
}
NAN_SETTER(ScriptProcessorNode::OnAudioProcessSetter) {
  Nan::HandleScope scope;

  ScriptProcessorNode *scriptProcessorNode = ObjectWrap::Unwrap<ScriptProcessorNode>(info.This());

  if (value->IsFunction()) {
    Local<Function> onAudioProcessLocal = Local<Function>::Cast(value);
    scriptProcessorNode->onAudioProcess.Reset(onAudioProcessLocal);
  } else {
    scriptProcessorNode->onAudioProcess.Reset();
  }
}
void ScriptProcessorNode::ProcessInAudioThread(lab::ContextRenderLock& r, vector<const float*> sources, vector<float*> destinations, size_t framesToProcess) {
  QueueOnMainThread(r, std::bind(ProcessInMainThread, this, sources, destinations, framesToProcess));
}
void ScriptProcessorNode::ProcessInMainThread(ScriptProcessorNode *self, vector<const float*> &sources, vector<float*> &destinations, size_t framesToProcess) {
  {
    Nan::HandleScope scope;

    if (!self->onAudioProcess.IsEmpty()) {
      Local<Function> onAudioProcessLocal = Nan::New(self->onAudioProcess);

      Local<Object> audioContextObj = Nan::New(self->context);
      AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

      Local<Array> sourcesArray = Nan::New<Array>(sources.size());
      for (size_t i = 0; i < sources.size(); i++) {
        const float *source = sources[i];
        Local<ArrayBuffer> sourceArrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), (void *)source, framesToProcess * sizeof(float));
        Local<Float32Array> sourceFloat32Array = Float32Array::New(sourceArrayBuffer, 0, framesToProcess);
        sourcesArray->Set(i, sourceFloat32Array);
      }
      Local<Function> audioBufferConstructorFn = Nan::New(self->audioBufferConstructor);
      Local<Value> argv1[] = {
        JS_INT((uint32_t)sources.size()),
        JS_INT((uint32_t)framesToProcess),
        JS_INT((uint32_t)audioContext->audioContext->sampleRate()),
        sourcesArray,
      };
      Local<Object> inputBuffer = audioBufferConstructorFn->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv1)/sizeof(argv1[0]), argv1).ToLocalChecked();

      Local<Array> destinationsArray = Nan::New<Array>(destinations.size());
      for (size_t i = 0; i < destinations.size(); i++) {
        float *destination = destinations[i];
        Local<ArrayBuffer> destinationArrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), destination, framesToProcess * sizeof(float));
        Local<Float32Array> destinationFloat32Array = Float32Array::New(destinationArrayBuffer, 0, framesToProcess);
        destinationsArray->Set(i, destinationFloat32Array);
      }
      Local<Value> argv2[] = {
        JS_INT((uint32_t)sources.size()),
        JS_INT((uint32_t)framesToProcess),
        JS_INT((uint32_t)audioContext->audioContext->sampleRate()),
        destinationsArray,
      };
      Local<Object> outputBuffer = audioBufferConstructorFn->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv2)/sizeof(argv2[0]), argv2).ToLocalChecked();

      Local<Function> audioProcessingEventConstructorFn = Nan::New(self->audioProcessingEventConstructor);
      Local<Value> argv3[] = {
        inputBuffer,
        outputBuffer,
        JS_INT((uint32_t)framesToProcess)
      };
      Local<Object> audioProcessingEventObj = audioProcessingEventConstructorFn->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv3)/sizeof(argv3[0]), argv3).ToLocalChecked();

      Local<Value> argv4[] = {
        audioProcessingEventObj,
      };
      onAudioProcessLocal->Call(Nan::Null(), sizeof(argv4)/sizeof(argv4[0]), argv4);

      for (size_t i = 0; i < sourcesArray->Length(); i++) {
        Local<Float32Array> sourceFloat32Array = Local<Float32Array>::Cast(sourcesArray->Get(i));
        sourceFloat32Array->Buffer()->Neuter();
      }
      for (size_t i = 0; i < destinationsArray->Length(); i++) {
        Local<Float32Array> destinationFloat32Array = Local<Float32Array>::Cast(destinationsArray->Get(i));
        destinationFloat32Array->Buffer()->Neuter();
      }
    } else {
      for (size_t i = 0; i < sources.size(); i++) {
        memcpy(destinations[i], sources[i], framesToProcess * sizeof(float));
      }
    }
  }
}

}

#include <ScriptProcessorNode.h>
#include <AudioContext.h>

namespace webaudio {

AudioProcessingEvent::AudioProcessingEvent(Local<Object> inputBuffer, Local<Object> outputBuffer) : inputBuffer(inputBuffer), outputBuffer(outputBuffer) {}
AudioProcessingEvent::~AudioProcessingEvent() {}
Local<Object> AudioProcessingEvent::Initialize(Isolate *isolate) {
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

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();

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

ScriptProcessorNode::ScriptProcessorNode(uint32_t bufferSize, uint32_t numberOfInputChannels, uint32_t numberOfOutputChannels) : bufferSize(bufferSize), numberOfInputChannels(numberOfInputChannels), numberOfOutputChannels(numberOfOutputChannels), bufferIndex(0) {
  for (size_t i = 0; i < numberOfInputChannels; i++) {
    inputBuffers.emplace_back((size_t)bufferSize);
    outputBuffers.emplace_back((size_t)bufferSize);
  }
}
ScriptProcessorNode::~ScriptProcessorNode() {}
Local<Object> ScriptProcessorNode::Initialize(Isolate *isolate, Local<Value> audioBufferCons, Local<Value> audioProcessingEventCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("ScriptProcessorNode"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  ScriptProcessorNode::InitializePrototype(proto);

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();

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
    info[3]->IsObject() && JS_OBJ(JS_OBJ(info[3])->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))
  ) {
    uint32_t bufferSize = TO_UINT32(info[0]);
    uint32_t numberOfInputChannels = TO_UINT32(info[1]);
    uint32_t numberOfOutputChannels = TO_UINT32(info[2]);

    if (numberOfInputChannels == numberOfOutputChannels) {
      Local<Object> audioContextObj = Local<Object>::Cast(info[3]);

      ScriptProcessorNode *scriptProcessorNode = new ScriptProcessorNode(bufferSize, numberOfInputChannels, numberOfOutputChannels);
      Local<Object> scriptProcessorNodeObj = info.This();
      scriptProcessorNode->Wrap(scriptProcessorNodeObj);

      scriptProcessorNode->context.Reset(audioContextObj);
      lab::ScriptProcessorNode *labScriptProcessorNode = new lab::ScriptProcessorNode(numberOfInputChannels, [scriptProcessorNode](lab::ContextRenderLock& r, vector<const float*> sources, vector<float*> destinations, size_t framesToProcess) {
        scriptProcessorNode->ProcessInAudioThread(r, sources, destinations, framesToProcess);
      });
      scriptProcessorNode->audioNode.reset(labScriptProcessorNode);

      Local<Function> audioBufferConstructor = Local<Function>::Cast(JS_OBJ(scriptProcessorNodeObj->Get(JS_STR("constructor")))->Get(JS_STR("AudioBuffer")));
      scriptProcessorNode->audioBufferConstructor.Reset(audioBufferConstructor);
      Local<Function> audioProcessingEventConstructor = Local<Function>::Cast(JS_OBJ(scriptProcessorNodeObj->Get(JS_STR("constructor")))->Get(JS_STR("AudioProcessingEvent")));
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
  for (size_t i = 0; i < sources.size(); i++) {
    const float *source = sources[i];
    float *inputBuffer = inputBuffers[i].data() + bufferIndex;
    memcpy(inputBuffer, source, framesToProcess * sizeof(float));
  }
  for (size_t i = 0; i < destinations.size(); i++) {
    float *destination = destinations[i];
    const float *outputBuffer = outputBuffers[i].data() + bufferIndex;
    memcpy(destination, outputBuffer, framesToProcess * sizeof(float));
  }
  bufferIndex += framesToProcess;

  if (bufferIndex >= bufferSize) {
    WebAudioAsync *webaudioAsync = getWebAudioAsync();
    webaudioAsync->QueueOnMainThread(r, std::bind(ProcessInMainThread, this));
    bufferIndex -= bufferSize;
  }
}
void ScriptProcessorNode::ProcessInMainThread(ScriptProcessorNode *self) {
  Nan::HandleScope scope;

  size_t framesToProcess = self->bufferSize;
  vector<vector<float>> &sources = self->inputBuffers;
  vector<vector<float>> &destinations = self->outputBuffers;

  Local<Array> outputAudioNodes = Nan::New(self->outputAudioNodes);
  size_t numOutputAudioNodes = outputAudioNodes->Length();
  bool connectedOutput = false;
  for (size_t i = 0; i < numOutputAudioNodes; i++) {
    if (TO_BOOL(outputAudioNodes->Get(i))) {
      connectedOutput = true;
      break;
    }
  }

  if (connectedOutput && !self->onAudioProcess.IsEmpty()) {
    Local<Function> onAudioProcessLocal = Nan::New(self->onAudioProcess);

    Local<Object> audioContextObj = Nan::New(self->context);
    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);

    Local<Array> sourcesArray = Nan::New<Array>(sources.size());
    for (size_t i = 0; i < sources.size(); i++) {
      const float *source = sources[i].data();
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
      float *destination = destinations[i].data();
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
    onAudioProcessLocal->Call(Isolate::GetCurrent()->GetCurrentContext(), Nan::Null(), sizeof(argv4)/sizeof(argv4[0]), argv4);

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
      memcpy(destinations[i].data(), sources[i].data(), framesToProcess * sizeof(float));
    }
  }
}

}

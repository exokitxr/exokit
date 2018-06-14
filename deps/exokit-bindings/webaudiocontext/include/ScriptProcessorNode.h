#ifndef _SCRIPT_PROCESSOR_NODE_H_
#define _SCRIPT_PROCESSOR_NODE_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include <functional>
#include "LabSound/extended/LabSound.h"
#include <defines.h>
#include <AudioNode.h>
#include <AudioBuffer.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {

class AudioProcessingEvent : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  static NAN_METHOD(New);

  AudioProcessingEvent(Local<Object> inputBuffer, Local<Object> outputBuffer);
  ~AudioProcessingEvent();

  static NAN_GETTER(InputBufferGetter);
  static NAN_GETTER(OutputBufferGetter);
  static NAN_GETTER(NumberOfInputChannelsGetter);
  static NAN_GETTER(NumberOfOutputChannelsGetter);

  Nan::Persistent<Object> inputBuffer;
  Nan::Persistent<Object> outputBuffer;
};

class ScriptProcessorNode : public AudioNode {
public:
  static Handle<Object> Initialize(Isolate *isolate, Local<Value> audioBufferCons, Local<Value> audioProcessingEventCons);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);

  ScriptProcessorNode(uint32_t bufferSize, uint32_t numberOfInputChannels, uint32_t numberOfOutputChannels);
  ~ScriptProcessorNode();

  static NAN_GETTER(OnAudioProcessGetter);
  static NAN_SETTER(OnAudioProcessSetter);
  void ProcessInAudioThread(lab::ContextRenderLock& r, vector<const float*> sources, vector<float*> destinations, size_t framesToProcess);
  static void ProcessInMainThread(ScriptProcessorNode *self);

  uint32_t bufferSize;
  uint32_t numberOfInputChannels;
  uint32_t numberOfOutputChannels;
  size_t bufferIndex;
  Nan::Persistent<Function> audioBufferConstructor;
  Nan::Persistent<Function> audioProcessingEventConstructor;
  Nan::Persistent<Function> onAudioProcess;
  vector<vector<float>> inputBuffers;
  vector<vector<float>> outputBuffers;
};

}

#endif

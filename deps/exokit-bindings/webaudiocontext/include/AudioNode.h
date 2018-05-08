#ifndef _AUDIO_NODE_H_
#define _AUDIO_NODE_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include <string>
#include "LabSound/extended/LabSound.h"
#include <defines.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {

class AudioNode : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(Connect);
  static NAN_METHOD(Disconnect);
  static NAN_GETTER(ContextGetter);
  static NAN_GETTER(NumberOfInputsGetter);
  static NAN_GETTER(NumberOfOutputsGetter);
  static NAN_GETTER(ChannelCountGetter);
  static NAN_GETTER(ChannelCountModeGetter);
  static NAN_GETTER(ChannelInterpretationGetter);

  AudioNode();
  ~AudioNode();

protected:
  Nan::Persistent<Object> context;
  shared_ptr<lab::AudioNode> audioNode;
  Nan::Persistent<Array> inputAudioNodes;
  Nan::Persistent<Array> outputAudioNodes;
};

}

#endif

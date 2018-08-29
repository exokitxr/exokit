#ifndef _AUDIO_DESTINATION_NODE_H_
#define _AUDIO_DESTINATION_NODE_H_

#include <v8.h>
#include <node.h>
#include <nan.h>
#include "LabSound/extended/LabSound.h"
#include <defines.h>
#include <AudioContext.h>
#include <AudioNode.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {

class AudioDestinationNode : public AudioNode {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(MaxChannelCountGetter);

  AudioDestinationNode();
  ~AudioDestinationNode();

protected:
};

}

#endif

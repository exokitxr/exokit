#ifndef _AUDIO_SOURCE_NODE_H_
#define _AUDIO_SOURCE_NODE_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include "LabSound/extended/LabSound.h"
#include <defines.h>
#include <Audio.h>
#include <AudioNode.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {

class AudioSourceNode : public AudioNode {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);

  AudioSourceNode();
  ~AudioSourceNode();

protected:
};

}

#endif

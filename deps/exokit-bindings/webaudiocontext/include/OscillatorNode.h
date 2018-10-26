#ifndef _OSCILLATOR_NODE_H_
#define _OSCILLATOR_NODE_H_

#include <v8.h>
#include <node.h>
#include <nan.h>
#include "LabSound/extended/LabSound.h"
#include <defines.h>
#include <AudioNode.h>
#include <AudioParam.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {

class OscillatorNode : public AudioNode {
public:
  static Handle<Object> Initialize(Isolate *isolate, Local<Value> audioParamCons);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(Start);
  static NAN_METHOD(Stop);

  OscillatorNode();
  ~OscillatorNode();

protected:
};

}

#endif

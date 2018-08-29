#ifndef _STEREO_PANNER_NODE_H_
#define _STEREO_PANNER_NODE_H_

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

class StereoPannerNode : public AudioNode {
public:
  static Handle<Object> Initialize(Isolate *isolate, Local<Value> audioParamCons);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);

  StereoPannerNode();
  ~StereoPannerNode();

protected:
};

}

#endif

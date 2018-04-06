#ifndef _PANNER_NODE_H_
#define _PANNER_NODE_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include "LabSound/extended/LabSound.h"
#include <defines.h>
#include <AudioNode.h>
#include <FakeAudioParam.h>
#include <AudioContext.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {

class PannerNode : public AudioNode {
public:
  static Handle<Object> Initialize(Isolate *isolate, Local<Value> fakeAudioParamCons);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(ConeInnerAngleGetter);
  static NAN_SETTER(ConeInnerAngleSetter);
  static NAN_GETTER(ConeOuterAngleGetter);
  static NAN_SETTER(ConeOuterAngleSetter);
  static NAN_GETTER(DistanceModelGetter);
  static NAN_SETTER(DistanceModelSetter);
  static NAN_GETTER(MaxDistanceGetter);
  static NAN_SETTER(MaxDistanceSetter);
  static NAN_GETTER(PanningModelGetter);
  static NAN_SETTER(PanningModelSetter);
  static NAN_GETTER(RefDistanceGetter);
  static NAN_SETTER(RefDistanceSetter);
  static NAN_GETTER(RolloffFactorGetter);
  static NAN_SETTER(RolloffFactorSetter);
  static NAN_METHOD(SetPosition);
  static NAN_METHOD(SetOrientation);
  static NAN_METHOD(SetPath);

  PannerNode();
  ~PannerNode();

protected:
  static string path;
};

}

#endif

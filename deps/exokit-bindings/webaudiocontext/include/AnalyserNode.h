#ifndef _ANALYSER_NODE_H_
#define _ANALYSER_NODE_H_

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

class AnalyserNode : public AudioNode {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(FftSizeGetter);
  static NAN_SETTER(FftSizeSetter);
  static NAN_GETTER(FrequencyBinCountGetter);
  static NAN_GETTER(MinDecibelsGetter);
  static NAN_GETTER(MaxDecibelsGetter);
  static NAN_GETTER(SmoothingTimeConstantGetter);
  static NAN_METHOD(GetFloatFrequencyData);
  static NAN_METHOD(GetByteFrequencyData);
  static NAN_METHOD(GetFloatTimeDomainData);
  static NAN_METHOD(GetByteTimeDomainData);

  AnalyserNode();
  ~AnalyserNode();

protected:
};

}

#endif

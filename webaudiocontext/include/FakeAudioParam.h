#ifndef _FAKE_AUDIO_PARAM_H_
#define _FAKE_AUDIO_PARAM_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include <functional>
#include "LabSound/extended/LabSound.h"
#include <defines.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {

class FakeAudioParam : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(Connect);
  static NAN_GETTER(DefaultValueGetter);
  static NAN_GETTER(MaxValueGetter);
  static NAN_GETTER(MinValueGetter);
  static NAN_GETTER(ValueGetter);
  static NAN_SETTER(ValueSetter);

  FakeAudioParam();
  ~FakeAudioParam();

protected:
  function<float()> getter;
  function<void(float)> setter;

  friend class AudioListener;
  friend class PannerNode;
};

}

#endif

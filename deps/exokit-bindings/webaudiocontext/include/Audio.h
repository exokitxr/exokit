#ifndef _HTML_AUDIO_ELEMENT_H_
#define _HTML_AUDIO_ELEMENT_H_

#include <v8.h>
#include <node.h>
#include <nan.h>
#include "LabSound/extended/LabSound.h"
#include <defines.h>
#include <AudioContext.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {

class Audio : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  void Load(uint8_t *bufferValue, size_t bufferLength);
  void Play();
  void Pause();

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(Load);
  static NAN_METHOD(Play);
  static NAN_METHOD(Pause);
  static NAN_GETTER(CurrentTimeGetter);
  static NAN_SETTER(CurrentTimeSetter);
  static NAN_GETTER(DurationGetter);
  static NAN_GETTER(LoopGetter);
  static NAN_SETTER(LoopSetter);

  Audio();
  ~Audio();

private:
  shared_ptr<lab::SampledAudioNode> audioNode;

  friend class AudioSourceNode;
};

}

#endif

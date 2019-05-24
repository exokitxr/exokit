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

class AudioContext;

class Audio : public ObjectWrap {
public:
  static Local<Object> Initialize(Isolate *isolate);

  void Play();
  void Pause();
  void Load(uint8_t *bufferValue, size_t bufferLength, Local<Function> cbFn);
  void Reparent(AudioContext *newAudioContext);

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(Load);
  static NAN_METHOD(Play);
  static NAN_METHOD(Pause);
  static NAN_GETTER(PausedGetter);
  static NAN_GETTER(CurrentTimeGetter);
  static NAN_SETTER(CurrentTimeSetter);
  static NAN_GETTER(DurationGetter);
  static NAN_GETTER(LoopGetter);
  static NAN_SETTER(LoopSetter);
  static NAN_GETTER(OnEndedGetter);
  static NAN_SETTER(OnEndedSetter);

  static void ProcessLoadInMainThread(Audio *self);
  static void ProcessInMainThread(Audio *self);

  Nan::Persistent<Function> onended;

  lab::AudioContext *audioContext;
  Nan::Persistent<Function> cbFn;
  shared_ptr<lab::AudioBus> audioBus;
  std::string error;

  Audio();
  ~Audio();

private:
  shared_ptr<lab::FinishableSourceNode> audioNode;

  friend class AudioSourceNode;
};

}

#endif

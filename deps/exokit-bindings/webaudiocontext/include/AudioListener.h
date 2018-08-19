#ifndef _AUDIO_LISTENER_H_
#define _AUDIO_LISTENER_H_

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

class AudioListener : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate, Local<Value> fakeAudioParamCons);

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(SetPosition);
  static NAN_METHOD(SetOrientation);

  AudioListener();
  ~AudioListener();

protected:
   lab::AudioListener *audioListener;
};

}

#endif

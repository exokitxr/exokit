#ifndef _MEDIA_STREAM_TRACK_H_
#define _MEDIA_STREAM_TRACK_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include "LabSound/extended/LabSound.h"
#include <defines.h>
#include <MicrophoneMediaStream.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {
  
class MediaStream;

class MediaStreamTrack : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);
  static NAN_GETTER(EnabledGetter);
  static NAN_GETTER(KindGetter);
  static NAN_GETTER(LabelGetter);
  static NAN_GETTER(MutedGetter);
  static NAN_SETTER(MutedSetter);
  static NAN_GETTER(ReadyStateGetter);
  static NAN_METHOD(Stop);

  MediaStreamTrack(MediaStream *mediaStream);
  ~MediaStreamTrack();

protected:
  std::shared_ptr<lab::AudioHardwareSourceNode> audioNode;
  MediaStream *mediaStream;
  bool live;
  bool muted;

  friend class AudioSourceNode;
};

}

#endif

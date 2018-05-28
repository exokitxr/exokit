#ifndef _MICROPHONE_MEDIA_STREAM_H_
#define _MICROPHONE_MEDIA_STREAM_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include "LabSound/extended/LabSound.h"
#include <defines.h>
#include <AudioSourceNode.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {
  
class MediaStreamTrack;
  
class MediaStream : public ObjectWrap {
  protected:
  std::shared_ptr<lab::AudioHardwareSourceNode> audioNode;
  
  friend class MediaStreamTrack;
};

class MicrophoneMediaStream : public MediaStream {
public:
  static Handle<Object> Initialize(Isolate *isolate, Local<Value> mediaStreamTrackCons);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(GetTracks);

  MicrophoneMediaStream();
  ~MicrophoneMediaStream();

protected:
  Nan::Persistent<Array> tracks;

  friend class AudioSourceNode;
};

}

#endif

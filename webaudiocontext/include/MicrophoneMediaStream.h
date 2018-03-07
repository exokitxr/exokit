#ifndef _MICROPHONE_MEDIA_STREAM_H_
#define _MICROPHONE_MEDIA_STREAM_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include "LabSound/extended/LabSound.h"
#include <defines.h>

using namespace std;
using namespace v8;
using namespace node;

namespace webaudio {

class MicrophoneMediaStream : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  static void InitializePrototype(Local<ObjectTemplate> proto);

protected:
  static NAN_METHOD(New);

  MicrophoneMediaStream();
  ~MicrophoneMediaStream();

protected:
  std::shared_ptr<lab::AudioHardwareSourceNode> audioNode;
};

}

#endif

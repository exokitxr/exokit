#include <AudioNode.h>
#include <AudioContext.h>

namespace webaudio {

AudioNode::AudioNode() {}

AudioNode::~AudioNode() {}

Handle<Object> AudioNode::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AudioNode"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

void AudioNode::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetMethod(proto, "connect", Connect);
  Nan::SetMethod(proto, "disconnect", Disconnect);
  Nan::SetAccessor(proto, JS_STR("context"), ContextGetter);
  Nan::SetAccessor(proto, JS_STR("numberOfInputsGetter"), NumberOfInputsGetter);
  Nan::SetAccessor(proto, JS_STR("numberOfOutputsGetter"), NumberOfOutputsGetter);
  Nan::SetAccessor(proto, JS_STR("channelCount"), ChannelCountGetter);
  Nan::SetAccessor(proto, JS_STR("channelCountMode"), ChannelCountModeGetter);
  Nan::SetAccessor(proto, JS_STR("channelInterpretation"), ChannelInterpretationGetter);
}

NAN_METHOD(AudioNode::New) {
  Nan::HandleScope scope;

  AudioNode *audioNode = new AudioNode();
  Local<Object> audioNodeObj = info.This();
  audioNode->Wrap(audioNodeObj);

  info.GetReturnValue().Set(audioNodeObj);
}

NAN_METHOD(AudioNode::Connect) {
  Nan::HandleScope scope;

  if (info[0]->IsObject()) {
    Local<Value> constructorName = info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"));

    if (
      constructorName->StrictEquals(JS_STR("AudioSourceNode")) ||
      constructorName->StrictEquals(JS_STR("AudioDestinationNode")) ||
      constructorName->StrictEquals(JS_STR("GainNode")) ||
      constructorName->StrictEquals(JS_STR("AnalyserNode")) ||
      constructorName->StrictEquals(JS_STR("PannerNode"))
    ) {
      unsigned int outputIndex = info[1]->IsNumber() ? info[1]->Uint32Value() : 0;
      unsigned int inputIndex = info[2]->IsNumber() ? info[2]->Uint32Value() : 0;

      AudioNode *audioNode = ObjectWrap::Unwrap<AudioNode>(info.This());
      shared_ptr<lab::AudioNode> srcAudioNode = audioNode->audioNode;

      AudioNode *argAudioNode = ObjectWrap::Unwrap<AudioNode>(Local<Object>::Cast(info[0]));
      shared_ptr<lab::AudioNode> dstAudioNode = argAudioNode->audioNode;

      Local<Object> audioContextObj = Nan::New(audioNode->context);
      AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);
      lab::AudioContext *labAudioContext = audioContext->audioContext;

      // try {
        labAudioContext->connect(dstAudioNode, srcAudioNode, outputIndex, inputIndex);
      /* } catch (const std::exception &e) {
        Nan::ThrowError(e.what());
        return;
      } catch (...) {
        Nan::ThrowError("unknown exception");
        return;
      } */

      info.GetReturnValue().Set(info[0]);
    } else {
      Nan::ThrowError("AudioNode::Connect: invalid arguments");
    }
  } else {
    Nan::ThrowError("AudioNode::Connect: invalid arguments");
  }
}

NAN_METHOD(AudioNode::Disconnect) {
  Nan::HandleScope scope;

  if (info.Length() == 0) {
    AudioNode *audioNode = ObjectWrap::Unwrap<AudioNode>(info.This());
    shared_ptr<lab::AudioNode> srcAudioNode = audioNode->audioNode;
    
    Local<Object> audioContextObj = Nan::New(audioNode->context);
    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);
    lab::AudioContext *labAudioContext = audioContext->audioContext;
    
    // try {
      labAudioContext->disconnect(nullptr, srcAudioNode);
    /* } catch (const std::exception &e) {
      Nan::ThrowError(e.what());
      return;
    } catch (...) {
      Nan::ThrowError("unknown exception");
      return;
    } */
  } else {
    if (info[0]->IsObject()) {
      Local<Value> constructorName = info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"));

      if (
        constructorName->StrictEquals(JS_STR("AudioSourceNode")) ||
        constructorName->StrictEquals(JS_STR("AudioDestinationNode")) ||
        constructorName->StrictEquals(JS_STR("GainNode")) ||
        constructorName->StrictEquals(JS_STR("AnalyserNode")) ||
        constructorName->StrictEquals(JS_STR("PannerNode"))
      ) {
        AudioNode *audioNode = ObjectWrap::Unwrap<AudioNode>(info.This());
        shared_ptr<lab::AudioNode> srcAudioNode = audioNode->audioNode;
        
        AudioNode *argAudioNode = ObjectWrap::Unwrap<AudioNode>(Local<Object>::Cast(info[0]));
        shared_ptr<lab::AudioNode> dstAudioNode = argAudioNode->audioNode;

        Local<Object> audioContextObj = Nan::New(audioNode->context);
        AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);
        lab::AudioContext *labAudioContext = audioContext->audioContext;
        // try {
          labAudioContext->disconnect(dstAudioNode, srcAudioNode);
        /* } catch (const std::exception &e) {
          Nan::ThrowError(e.what());
          return;
        } catch (...) {
          Nan::ThrowError("unknown exception");
          return;
        } */

        info.GetReturnValue().Set(info[0]);
      } else {
        Nan::ThrowError("AudioNode::Disconnect: invalid arguments");
      }
    } else {
      Nan::ThrowError("AudioNode::Disconnect: invalid arguments");
    }
  }
}

NAN_GETTER(AudioNode::ContextGetter) {
  Nan::HandleScope scope;

  AudioNode *audioNode = ObjectWrap::Unwrap<AudioNode>(info.This());
  if (!audioNode->context.IsEmpty()) {
    info.GetReturnValue().Set(Nan::New(audioNode->context));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_GETTER(AudioNode::NumberOfInputsGetter) {
  Nan::HandleScope scope;

  AudioNode *audioNode = ObjectWrap::Unwrap<AudioNode>(info.This());
  
  unsigned int numberOfInputs = audioNode->audioNode->numberOfInputs();

  info.GetReturnValue().Set(JS_INT(numberOfInputs));
}

NAN_GETTER(AudioNode::NumberOfOutputsGetter) {
  Nan::HandleScope scope;

  AudioNode *audioNode = ObjectWrap::Unwrap<AudioNode>(info.This());

  unsigned int numberOfOutputs = audioNode->audioNode->numberOfOutputs();

  info.GetReturnValue().Set(JS_INT(numberOfOutputs));
}

NAN_GETTER(AudioNode::ChannelCountGetter) {
  Nan::HandleScope scope;

  AudioNode *audioNode = ObjectWrap::Unwrap<AudioNode>(info.This());

  unsigned int channelCount = audioNode->audioNode->channelCount();
  
  info.GetReturnValue().Set(JS_INT(channelCount));
}

NAN_GETTER(AudioNode::ChannelCountModeGetter) {
  Nan::HandleScope scope;

  AudioNode *audioNode = ObjectWrap::Unwrap<AudioNode>(info.This());

  lab::ChannelCountMode channelCountMode = audioNode->audioNode->channelCountMode();
  Local<String> result;
  switch (channelCountMode) {
    case lab::ChannelCountMode::Max: {
      result = Nan::New<String>("max").ToLocalChecked();
      break;
    }
    case lab::ChannelCountMode::ClampedMax: {
      result = Nan::New<String>("clamped-max").ToLocalChecked();
      break;
    }
    case lab::ChannelCountMode::Explicit: {
      result = Nan::New<String>("explicit").ToLocalChecked();
      break;
    }
    default: {
      result = Nan::New<String>("").ToLocalChecked();
      break;
    }
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(AudioNode::ChannelInterpretationGetter) {
  Nan::HandleScope scope;

  AudioNode *audioNode = ObjectWrap::Unwrap<AudioNode>(info.This());

  lab::ChannelInterpretation channelInterpretation = audioNode->audioNode->channelInterpretation();
  Local<String> result;
  switch (channelInterpretation) {
    case lab::ChannelInterpretation::Speakers: {
      result = Nan::New<String>("speakers").ToLocalChecked();
      break;
    }
    case lab::ChannelInterpretation::Discrete: {
      result = Nan::New<String>("discrete").ToLocalChecked();
      break;
    }
    default: {
      result = Nan::New<String>("").ToLocalChecked();
      break;
    }
  }

  info.GetReturnValue().Set(result);
}

}

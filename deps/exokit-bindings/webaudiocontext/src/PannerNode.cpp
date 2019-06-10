#include <PannerNode.h>

namespace webaudio {

enum ModelType {
  ModelLinear = 0,
  ModelInverse = 1,
  ModelExponential = 2,
};

PannerNode::PannerNode() {}

PannerNode::~PannerNode() {}

Local<Object> PannerNode::Initialize(Isolate *isolate, Local<Value> fakeAudioParamCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("PannerNode"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  AudioNode::InitializePrototype(proto);
  PannerNode::InitializePrototype(proto);

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();

  Nan::SetMethod(ctorFn, "setPath", SetPath);
  ctorFn->Set(JS_STR("FakeAudioParam"), fakeAudioParamCons);

  return scope.Escape(ctorFn);
}

void PannerNode::InitializePrototype(Local<ObjectTemplate> proto) {
  Nan::SetAccessor(proto, JS_STR("coneInnerAngle"), ConeInnerAngleGetter, ConeInnerAngleSetter);
  Nan::SetAccessor(proto, JS_STR("coneOuterAngle"), ConeOuterAngleGetter, ConeOuterAngleSetter);
  Nan::SetAccessor(proto, JS_STR("distanceModel"), DistanceModelGetter, DistanceModelSetter);
  Nan::SetAccessor(proto, JS_STR("maxDistance"), MaxDistanceGetter, MaxDistanceSetter);
  Nan::SetAccessor(proto, JS_STR("panningModel"), PanningModelGetter, PanningModelSetter);
  Nan::SetAccessor(proto, JS_STR("refDistance"), RefDistanceGetter, RefDistanceSetter);
  Nan::SetAccessor(proto, JS_STR("rolloffFactor"), RolloffFactorGetter, RolloffFactorSetter);
  Nan::SetMethod(proto, "setPosition", SetPosition);
  Nan::SetMethod(proto, "setOrientation", SetOrientation);
}

NAN_METHOD(PannerNode::New) {
  // Nan::HandleScope scope;

  if (info[0]->IsObject() && JS_OBJ(JS_OBJ(info[0])->Get(JS_STR("constructor")))->Get(JS_STR("name"))->StrictEquals(JS_STR("AudioContext"))) {
    Local<Object> audioContextObj = Local<Object>::Cast(info[0]);

    PannerNode *pannerNode = new PannerNode();
    Local<Object> pannerNodeObj = info.This();
    pannerNode->Wrap(pannerNodeObj);

    AudioContext *audioContext = ObjectWrap::Unwrap<AudioContext>(audioContextObj);
    shared_ptr<lab::PannerNode> labPannerNode = make_shared<lab::PannerNode>(audioContext->audioContext->sampleRate(), PannerNode::path);

    pannerNode->context.Reset(audioContextObj);
    pannerNode->audioNode = labPannerNode;

    Local<Function> fakeAudioParamConstructor = Local<Function>::Cast(JS_OBJ(pannerNodeObj->Get(JS_STR("constructor")))->Get(JS_STR("FakeAudioParam")));

    Local<Object> positionXAudioParamObj = fakeAudioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
    FakeAudioParam *positionXAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(positionXAudioParamObj);
    positionXAudioParam->getter = [=]() -> float { return labPannerNode->position().x; };
    positionXAudioParam->setter = [=](float x) -> void { lab::FloatPoint3D position = labPannerNode->position(); labPannerNode->setPosition(x, position.y, position.z); };
    pannerNodeObj->Set(JS_STR("positionX"), positionXAudioParamObj);

    Local<Object> positionYAudioParamObj = fakeAudioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
    FakeAudioParam *positionYAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(positionYAudioParamObj);
    positionYAudioParam->getter = [=]() -> float { return labPannerNode->position().y; };
    positionYAudioParam->setter = [=](float y) -> void { lab::FloatPoint3D position = labPannerNode->position(); labPannerNode->setPosition(position.x, y, position.z); };
    pannerNodeObj->Set(JS_STR("positionY"), positionYAudioParamObj);

    Local<Object> positionZAudioParamObj = fakeAudioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
    FakeAudioParam *positionZAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(positionZAudioParamObj);
    positionZAudioParam->getter = [=]() -> float { return labPannerNode->position().z; };
    positionZAudioParam->setter = [=](float z) -> void { lab::FloatPoint3D position = labPannerNode->position(); labPannerNode->setPosition(position.x, position.y, z); };
    pannerNodeObj->Set(JS_STR("positionZ"), positionZAudioParamObj);

    Local<Object> orientationXAudioParamObj = fakeAudioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
    FakeAudioParam *orientationXAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(orientationXAudioParamObj);
    orientationXAudioParam->getter = [=]() -> float { return labPannerNode->orientation().x; };
    orientationXAudioParam->setter = [=](float x) -> void { lab::FloatPoint3D orientation = labPannerNode->orientation(); labPannerNode->setOrientation(x, orientation.y, orientation.z); };
    pannerNodeObj->Set(JS_STR("orientationX"), orientationXAudioParamObj);

    Local<Object> orientationYAudioParamObj = fakeAudioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
    FakeAudioParam *orientationYAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(orientationYAudioParamObj);
    orientationYAudioParam->getter = [=]() -> float { return labPannerNode->orientation().y; };
    orientationYAudioParam->setter = [=](float y) -> void { lab::FloatPoint3D orientation = labPannerNode->orientation(); labPannerNode->setOrientation(orientation.x, y, orientation.z); };
    pannerNodeObj->Set(JS_STR("orientationY"), orientationYAudioParamObj);

    Local<Object> orientationZAudioParamObj = fakeAudioParamConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
    FakeAudioParam *orientationZAudioParam = ObjectWrap::Unwrap<FakeAudioParam>(orientationZAudioParamObj);
    orientationZAudioParam->getter = [=]() -> float { return labPannerNode->orientation().z; };
    orientationZAudioParam->setter = [=](float z) -> void { lab::FloatPoint3D orientation = labPannerNode->orientation(); labPannerNode->setOrientation(orientation.x, orientation.y, z); };
    pannerNodeObj->Set(JS_STR("orientationZ"), orientationZAudioParamObj);

    info.GetReturnValue().Set(pannerNodeObj);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_GETTER(PannerNode::ConeInnerAngleGetter) {
  Nan::HandleScope scope;

  PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
  shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

  float coneInnerAngle = labPannerNode->coneInnerAngle();

  info.GetReturnValue().Set(JS_NUM(coneInnerAngle));
}

NAN_SETTER(PannerNode::ConeInnerAngleSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber()) {
    PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
    shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

    float newValue = TO_FLOAT(value);
    labPannerNode->setConeInnerAngle(newValue);
  } else {
    Nan::ThrowError("value: invalid arguments");
  }
}

NAN_GETTER(PannerNode::ConeOuterAngleGetter) {
  Nan::HandleScope scope;

  PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
  shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

  float coneOuterAngle = labPannerNode->coneOuterAngle();

  info.GetReturnValue().Set(JS_NUM(coneOuterAngle));
}

NAN_SETTER(PannerNode::ConeOuterAngleSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber()) {
    PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
    shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

    float newValue = TO_FLOAT(value);
    labPannerNode->setConeOuterAngle(newValue);
  } else {
    Nan::ThrowError("value: invalid arguments");
  }
}

NAN_GETTER(PannerNode::DistanceModelGetter) {
  Nan::HandleScope scope;

  PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
  shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

  Local<String> result;
  ModelType distanceModel = (ModelType)labPannerNode->distanceModel();
  switch (distanceModel) {
    case ModelType::ModelLinear: {
      result = Nan::New<String>("linear").ToLocalChecked();
      break;
    }
    case ModelType::ModelInverse: {
      result = Nan::New<String>("inverse").ToLocalChecked();
      break;
    }
    case ModelType::ModelExponential: {
      result = Nan::New<String>("exponential").ToLocalChecked();
      break;
    }
    default: {
      result = Nan::New<String>("").ToLocalChecked();
      break;
    }
  }

  info.GetReturnValue().Set(result);
}

NAN_SETTER(PannerNode::DistanceModelSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
    shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

    Nan::Utf8String valueUtf8(Local<String>::Cast(value));
    string valueString(*valueUtf8, valueUtf8.length());

    ModelType distanceModel;
    if (valueString == "linear") {
      distanceModel = ModelType::ModelLinear;
    } else if (valueString == "inverse") {
      distanceModel = ModelType::ModelInverse;
    } else if (valueString == "exponential") {
      distanceModel = ModelType::ModelExponential;
    } else {
      distanceModel = ModelType::ModelLinear;
    }

    labPannerNode->setDistanceModel(distanceModel);
  } else {
    Nan::ThrowError("value: invalid arguments");
  }
}

NAN_GETTER(PannerNode::MaxDistanceGetter) {
  Nan::HandleScope scope;

  PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
  shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

  float maxDistance = labPannerNode->maxDistance();

  info.GetReturnValue().Set(JS_NUM(maxDistance));
}

NAN_SETTER(PannerNode::MaxDistanceSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber()) {
    PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
    shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

    float newValue = TO_FLOAT(value);
    labPannerNode->setMaxDistance(newValue);
  } else {
    Nan::ThrowError("value: invalid arguments");
  }
}

NAN_GETTER(PannerNode::PanningModelGetter) {
  Nan::HandleScope scope;

  PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
  shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

  Local<String> result;
  lab::PanningMode panningModel = labPannerNode->panningModel();
  switch (panningModel) {
    case lab::PanningMode::EQUALPOWER: {
      result = Nan::New<String>("equalpower").ToLocalChecked();
      break;
    }
    case lab::PanningMode::HRTF: {
      result = Nan::New<String>("HRTF").ToLocalChecked();
      break;
    }
    default: {
      result = Nan::New<String>("").ToLocalChecked();
      break;
    }
  }

  info.GetReturnValue().Set(result);
}

NAN_SETTER(PannerNode::PanningModelSetter) {
  Nan::HandleScope scope;

  if (value->IsString()) {
    PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
    shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

    Nan::Utf8String valueUtf8(Local<String>::Cast(value));
    string valueString(*valueUtf8, valueUtf8.length());

    lab::PanningMode panningModel;
    if (valueString == "equalpower") {
      panningModel = lab::PanningMode::EQUALPOWER;
    } else if (valueString == "HRTF") {
      panningModel = lab::PanningMode::HRTF;
    } else {
      panningModel = lab::PanningMode::EQUALPOWER;
    }

    labPannerNode->setPanningModel(panningModel);
  } else {
    Nan::ThrowError("value: invalid arguments");
  }
}

NAN_GETTER(PannerNode::RefDistanceGetter) {
  Nan::HandleScope scope;

  PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
  shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

  float refDistance = labPannerNode->refDistance();

  info.GetReturnValue().Set(JS_NUM(refDistance));
}

NAN_SETTER(PannerNode::RefDistanceSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber()) {
    PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
    shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

    float newValue = TO_FLOAT(value);
    labPannerNode->setRefDistance(newValue);
  } else {
    Nan::ThrowError("value: invalid arguments");
  }
}

NAN_GETTER(PannerNode::RolloffFactorGetter) {
  Nan::HandleScope scope;

  PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
  shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

  float rolloffFactor = labPannerNode->rolloffFactor();

  info.GetReturnValue().Set(JS_NUM(rolloffFactor));
}

NAN_SETTER(PannerNode::RolloffFactorSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber()) {
    PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
    shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

    float newValue = TO_FLOAT(value);
    labPannerNode->setRolloffFactor(newValue);
  } else {
    Nan::ThrowError("value: invalid arguments");
  }
}

NAN_METHOD(PannerNode::SetPosition) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
    shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

    float x = TO_FLOAT(info[0]);
    float y = TO_FLOAT(info[1]);
    float z = TO_FLOAT(info[2]);

    labPannerNode->setPosition(x, y, z);
  } else {
    Nan::ThrowError("AnalyserNode::GetFloatFrequencyData: invalid arguments");
  }
}

NAN_METHOD(PannerNode::SetOrientation) {
  Nan::HandleScope scope;

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    PannerNode *pannerNode = ObjectWrap::Unwrap<PannerNode>(info.This());
    shared_ptr<lab::PannerNode> labPannerNode = *(shared_ptr<lab::PannerNode> *)(&pannerNode->audioNode);

    float x = TO_FLOAT(info[0]);
    float y = TO_FLOAT(info[1]);
    float z = TO_FLOAT(info[2]);

    labPannerNode->setOrientation(x, y, z);
  } else {
    Nan::ThrowError("AnalyserNode::GetFloatFrequencyData: invalid arguments");
  }
}

NAN_METHOD(PannerNode::SetPath) {
  Nan::HandleScope scope;

  if (info[0]->IsString()) {
    Nan::Utf8String pathValue(Local<String>::Cast(info[0]));

    PannerNode::path = *pathValue;
  } else {
    Nan::ThrowError("PannerNode::SetPath: invalid arguments");
  }
}

string PannerNode::path;

}

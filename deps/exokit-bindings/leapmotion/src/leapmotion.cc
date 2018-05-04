#include <leapmotion.h>

using namespace v8;
using namespace std;

namespace lm {

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};

void ListenerImpl::onInit(const Leap::Controller& controller) {
  // std::cout << "Initialized" << std::endl;
}

void ListenerImpl::onConnect(const Leap::Controller& controller) {
  // std::cout << "Connected" << std::endl;
}

void ListenerImpl::onDisconnect(const Leap::Controller& controller) {
  // Note: not dispatched when running in a debugger.
  // std::cout << "Disconnected" << std::endl;
}

void ListenerImpl::onExit(const Leap::Controller& controller) {
  // std::cout << "Exited" << std::endl;
}

void ListenerImpl::onFrame(const Leap::Controller& controller) {
  // Get the most recent frame and report some basic information
  const Leap::Frame frame = controller.frame();
  /* std::cout << "Frame id: " << frame.id()
            << ", timestamp: " << frame.timestamp()
            << ", hands: " << frame.hands().count()
            << ", extended fingers: " << frame.fingers().extended().count() << std::endl; */

  Leap::HandList hands = frame.hands();
  for (Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    // Get the first hand
    const Leap::Hand hand = *hl;
    std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
    /* std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
              << ", palm position: " << hand.palmPosition() << std::endl; */
    // Get the hand's normal vector and direction
    const Leap::Vector normal = hand.palmNormal();
    const Leap::Vector direction = hand.direction();

    // Calculate the hand's pitch, roll, and yaw angles
    /* std::cout << std::string(2, ' ') <<  "pitch: " << direction.pitch() * Leap::RAD_TO_DEG << " degrees, "
              << "roll: " << normal.roll() * Leap::RAD_TO_DEG << " degrees, "
              << "yaw: " << direction.yaw() * Leap::RAD_TO_DEG << " degrees" << std::endl; */

    // Get the Arm bone
    Leap::Arm arm = hand.arm();
    /* std::cout << std::string(2, ' ') <<  "Arm direction: " << arm.direction()
              << " wrist position: " << arm.wristPosition()
              << " elbow position: " << arm.elbowPosition() << std::endl; */

    // Get fingers
    const Leap::FingerList fingers = hand.fingers();
    for (Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      const Leap::Finger finger = *fl;
      /* std::cout << std::string(4, ' ') <<  fingerNames[finger.type()]
                << " finger, id: " << finger.id()
                << ", length: " << finger.length()
                << "mm, width: " << finger.width() << std::endl; */

      // Get finger bones
      for (int b = 0; b < 4; ++b) {
        Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
        Leap::Bone bone = finger.bone(boneType);
        /* std::cout << std::string(6, ' ') <<  boneNames[boneType]
                  << " bone, start: " << bone.prevJoint()
                  << ", end: " << bone.nextJoint()
                  << ", direction: " << bone.direction() << std::endl; */
      }
    }
  }

  /* if (!frame.hands().isEmpty()) {
    std::cout << std::endl;
  } */

}

void ListenerImpl::onFocusGained(const Leap::Controller& controller) {
  // std::cout << "Focus Gained" << std::endl;
}

void ListenerImpl::onFocusLost(const Leap::Controller& controller) {
  // std::cout << "Focus Lost" << std::endl;
}

void ListenerImpl::onDeviceChange(const Leap::Controller& controller) {
  // std::cout << "Device Changed" << std::endl;
  const Leap::DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
    /* std::cout << "id: " << devices[i].toString() << std::endl;
    std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
    std::cout << "  isSmudged:" << (devices[i].isSmudged() ? "true" : "false") << std::endl;
    std::cout << "  isLightingBad:" << (devices[i].isLightingBad() ? "true" : "false") << std::endl; */
  }
}

void ListenerImpl::onServiceConnect(const Leap::Controller& controller) {
  // std::cout << "Service Connected" << std::endl;
}

void ListenerImpl::onServiceDisconnect(const Leap::Controller& controller) {
  // std::cout << "Service Disconnected" << std::endl;
}

void ListenerImpl::onServiceChange(const Leap::Controller& controller) {
  // std::cout << "Service Changed" << std::endl;
}

void ListenerImpl::onDeviceFailure(const Leap::Controller& controller) {
  /* std::cout << "Device Error" << std::endl;
  const Leap::FailedDeviceList devices = controller.failedDevices();

  for (Leap::FailedDeviceList::const_iterator dl = devices.begin(); dl != devices.end(); ++dl) {
    const Leap::FailedDevice device = *dl;
    std::cout << "  PNP ID:" << device.pnpId();
    std::cout << "    Failure type:" << device.failure();
  } */
}

void ListenerImpl::onLogMessage(const Leap::Controller&, Leap::MessageSeverity s, int64_t t, const char* msg) {
  /* switch (s) {
  case Leap::MESSAGE_CRITICAL:
    std::cout << "[Critical]";
    break;
  case Leap::MESSAGE_WARNING:
    std::cout << "[Warning]";
    break;
  case Leap::MESSAGE_INFORMATION:
    std::cout << "[Info]";
    break;
  case Leap::MESSAGE_UNKNOWN:
    std::cout << "[Unknown]";
  }
  std::cout << "[" << t << "] ";
  std::cout << msg << std::endl; */
}

LMContext::LMContext() {
  controller.addListener(listener);
}

LMContext::~LMContext() {}

Handle<Object> LMContext::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("LMContext"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "WaitGetPoses", WaitGetPoses);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_METHOD(LMContext::New) {
  Nan::HandleScope scope;

  Local<Object> lmContextObj = info.This();
  LMContext *lmContext = new LMContext();
  lmContext->Wrap(lmContextObj);

  info.GetReturnValue().Set(lmContextObj);
}

NAN_METHOD(LMContext::WaitGetPoses) {
  LMContext *lmContext = ObjectWrap::Unwrap<LMContext>(info.This());

  if (info[0]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[0]);

    for (size_t i = 0; i < 2; i++) {
      Local<Float32Array> handFloat32Array = Local<Float32Array>::Cast(array->Get(i));
      for (size_t j = 0; j < 6; j++) {
        handFloat32Array->Set(j, JS_NUM(0));
      }
    }

    Leap::Frame frame = lmContext->controller.frame();
    Leap::HandList hands = frame.hands();
    size_t numHands = hands.count();
    for (size_t i = 0; i < numHands && i < 2; i++) {
      const Leap::Hand hand = hands[i];
      const Leap::Vector position = hand.palmPosition();
      const Leap::Vector normal = hand.palmNormal();

      Local<Float32Array> handFloat32Array = Local<Float32Array>::Cast(array->Get(i));
      handFloat32Array->Set(0, JS_NUM(-position.x / 1000.0));
      handFloat32Array->Set(1, JS_NUM(-position.z / 1000.0));
      handFloat32Array->Set(2, JS_NUM(-position.y / 1000.0));
      handFloat32Array->Set(3, JS_NUM(-normal.x));
      handFloat32Array->Set(4, JS_NUM(-normal.z));
      handFloat32Array->Set(5, JS_NUM(-normal.y));
      
      const Leap::FingerList fingers = hand.fingers();
      size_t numFingers = fingers.count();
      for (size_t i = 0 ; i < numFingers; i++) {
        const Leap::Finger finger = fingers[i];

        for (int b = 0; b < 4; b++) {
          Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
          Leap::Bone bone = finger.bone(boneType);
          Leap::Vector center = bone.center();
          Leap::Vector centerV(-center.x / 1000.0, -center.z / 1000.0, -center.y / 1000.0);
          Leap::Vector direction = bone.direction();
          Leap::Vector directionV(-direction.x, -direction.z, -direction.y);
          float length = bone.length() / 1000.0;
          
          Leap::Vector start = centerV + (directionV * length/2);
          Leap::Vector end = centerV - (directionV * length/2);
          
          size_t baseIndex = (1 + (i * 4) + b) * (3 + 3);
          handFloat32Array->Set(baseIndex + 0, JS_NUM(start.x));
          handFloat32Array->Set(baseIndex + 1, JS_NUM(start.y));
          handFloat32Array->Set(baseIndex + 2, JS_NUM(start.z));
          handFloat32Array->Set(baseIndex + 3, JS_NUM(end.x));
          handFloat32Array->Set(baseIndex + 4, JS_NUM(end.y));
          handFloat32Array->Set(baseIndex + 5, JS_NUM(end.z));
        }
      }
    }
  } else {
    Nan::ThrowError("LMContext::WaitGetPoses: invalid arguments");
  }
}

}

Handle<Object> makeLm() {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  return scope.Escape(lm::LMContext::Initialize(isolate));
}
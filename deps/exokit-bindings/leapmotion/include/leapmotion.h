#ifndef _LEAPMOTION_H_
#define _LEAPMOTION_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include <defines.h>
#include <Leap.h>
#include <LeapMath.h>
#include <cstring>

using namespace v8;
using namespace node;

namespace lm {

class ListenerImpl : public Leap::Listener {
public:
  virtual void onInit(const Leap::Controller&);
  virtual void onConnect(const Leap::Controller&);
  virtual void onDisconnect(const Leap::Controller&);
  virtual void onExit(const Leap::Controller&);
  virtual void onFrame(const Leap::Controller&);
  virtual void onFocusGained(const Leap::Controller&);
  virtual void onFocusLost(const Leap::Controller&);
  virtual void onDeviceChange(const Leap::Controller&);
  virtual void onServiceConnect(const Leap::Controller&);
  virtual void onServiceDisconnect(const Leap::Controller&);
  virtual void onServiceChange(const Leap::Controller&);
  virtual void onDeviceFailure(const Leap::Controller&);
  virtual void onLogMessage(const Leap::Controller&, Leap::MessageSeverity severity, int64_t timestamp, const char* msg);
};

class LMContext : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  LMContext();
  ~LMContext();

  static NAN_METHOD(New);
  static NAN_METHOD(WaitGetPoses);

  ListenerImpl listener;
  Leap::Controller controller;
};

}

Handle<Object> makeLm();

#endif

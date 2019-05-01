#ifndef _ZED_H_
#define _ZED_H_

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>

#include <chrono>

#include <sl/Camera.hpp>

using namespace v8;
using namespace node;

namespace zed {

class Zed : public ObjectWrap {
public:
  static Local<Object> Initialize(Isolate *isolate);

protected:
  Zed();
  ~Zed();

  static NAN_METHOD(New);
  static NAN_METHOD(RequestPresent);
  static NAN_METHOD(WaitGetPoses);
};

}

Local<Object> makeZed();

#endif

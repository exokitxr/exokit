/*
 * webgl.h
 *
 *  Created on: Dec 13, 2011
 *      Author: ngk437
 */

#ifndef _MAGICLEAP_H_
#define _MAGICLEAP_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>
#include <defines.h>
#include <glfw.h>
#include <ml_graphics.h>
#include <ml_head_tracking.h>
#include <ml_perception.h>
#include <ml_lifecycle.h>
#include <ml_logging.h>

using namespace v8;
using namespace node;

namespace ml {
  
struct application_context_t {
  int dummy_value;
};
  
class MLContext : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  MLContext();
  ~MLContext();

  static NAN_METHOD(New);
  static NAN_METHOD(Init);
  static NAN_METHOD(Update);

protected:
  struct application_context_t application_context;
  MLHandle graphics_client;
};

}

Handle<Object> makeMl();

#endif

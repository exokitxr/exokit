#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <stdio.h>

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>

using namespace v8;
using namespace node;

namespace console {

NAN_METHOD(Log);
Local<Object> Initialize(Isolate *isolate);
  
};

#endif

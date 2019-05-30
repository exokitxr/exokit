#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdio.h>
#include <string>
#include <map>
#include <mutex>

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <defines.h>

using namespace v8;
using namespace node;

namespace cache {

extern std::map<std::string, std::vector<uint8_t>> items;
extern std::mutex mutex;

NAN_METHOD(Get);
NAN_METHOD(Set);
Local<Object> Initialize(Isolate *isolate);
  
};

#endif

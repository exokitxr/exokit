#include <cache.h>

namespace cache {

std::map<std::string, std::string> items;
std::mutex mutex;

NAN_METHOD(Get) {
  Local<String> keyValue = Local<String>::Cast(info[0]);
  Nan::Utf8String keyUtf8String(keyValue);
  std::string key(*keyUtf8String, keyUtf8String.length());

  std::string *value;
  {
    std::lock_guard<std::mutex> lock(mutex);

    auto iter = items.find(key);
    if (iter != items.end()) {
      value = &iter->second;
    } else {
      value = nullptr;
    }
  }
  if (value) {
    info.GetReturnValue().Set(JS_STR(*value));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}
  
NAN_METHOD(Set) {
  Local<String> keyValue = Local<String>::Cast(info[0]);
  Nan::Utf8String keyUtf8String(keyValue);
  std::string key(*keyUtf8String, keyUtf8String.length());

  Local<String> valueValue = Local<String>::Cast(info[1]);
  Nan::Utf8String valueUtf8String(valueValue);
  std::string value(*valueUtf8String, valueUtf8String.length());

  {
    std::lock_guard<std::mutex> lock(mutex);

    items[std::move(key)] = std::move(value);
  }
}

Local<Object> Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  Local<Object> result = Nan::New<Object>();

  Local<FunctionTemplate> getFnTemplate = Nan::New<FunctionTemplate>(Get);
  Local<Function> getFn = Nan::GetFunction(getFnTemplate).ToLocalChecked();
  result->Set(JS_STR("Get"), getFn);
  Local<FunctionTemplate> setFnTemplate = Nan::New<FunctionTemplate>(Set);
  Local<Function> setFn = Nan::GetFunction(setFnTemplate).ToLocalChecked();
  result->Set(JS_STR("Set"), setFn);

  return scope.Escape(result);
}

};
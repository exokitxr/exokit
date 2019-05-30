#include <cache.h>

namespace cache {

std::map<std::string, std::vector<uint8_t>> items;
std::mutex mutex;

NAN_METHOD(Get) {
  Local<String> keyValue = Local<String>::Cast(info[0]);
  Nan::Utf8String keyUtf8String(keyValue);
  std::string key(*keyUtf8String, keyUtf8String.length());

  std::vector<uint8_t> *valuePtr;
  {
    std::lock_guard<std::mutex> lock(mutex);

    auto iter = items.find(key);
    if (iter != items.end()) {
      valuePtr = &iter->second;
    } else {
      valuePtr = nullptr;
    }
  }
  if (valuePtr) {
    Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), valuePtr->data(), valuePtr->size());
    Local<Uint8Array> uint8Array = Uint8Array::New(arrayBuffer, 0, arrayBuffer->ByteLength());
    info.GetReturnValue().Set(uint8Array);
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}
  
NAN_METHOD(Set) {
  std::vector<uint8_t> value;
  if (info[1]->IsArrayBuffer()) {
    Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(info[1]);
    value = std::vector<uint8_t>(arrayBuffer->ByteLength());
    memcpy(value.data(), (uint8_t *)arrayBuffer->GetContents().Data(), value.size());
  } else if (info[1]->IsTypedArray()) {
    Local<TypedArray> typedArray = Local<TypedArray>::Cast(info[1]);
    value = std::vector<uint8_t>(typedArray->ByteLength());
    memcpy(value.data(), (uint8_t *)typedArray->Buffer()->GetContents().Data() + typedArray->ByteOffset(), value.size());
  } else {
    return Nan::ThrowError("NativeCache::Set: invalid arguments");
  }
  
  Local<String> keyValue = Local<String>::Cast(info[0]);
  Nan::Utf8String keyUtf8String(keyValue);
  std::string key(*keyUtf8String, keyUtf8String.length());

  {
    std::lock_guard<std::mutex> lock(mutex);

    items[std::move(key)] = std::move(value);
  }
}

NAN_METHOD(Delete) {
  Local<String> keyValue = Local<String>::Cast(info[0]);
  Nan::Utf8String keyUtf8String(keyValue);
  std::string key(*keyUtf8String, keyUtf8String.length());

  {
    std::lock_guard<std::mutex> lock(mutex);

    items.erase(std::move(key));
  }
}

Local<Object> Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  Local<Object> result = Nan::New<Object>();

  Local<FunctionTemplate> getFnTemplate = Nan::New<FunctionTemplate>(Get);
  Local<Function> getFn = Nan::GetFunction(getFnTemplate).ToLocalChecked();
  result->Set(JS_STR("get"), getFn);
  Local<FunctionTemplate> setFnTemplate = Nan::New<FunctionTemplate>(Set);
  Local<Function> setFn = Nan::GetFunction(setFnTemplate).ToLocalChecked();
  result->Set(JS_STR("set"), setFn);
  Local<FunctionTemplate> deleteFnTemplate = Nan::New<FunctionTemplate>(Delete);
  Local<Function> deleteFn = Nan::GetFunction(deleteFnTemplate).ToLocalChecked();
  result->Set(JS_STR("delete"), setFn);

  return scope.Escape(result);
}

};
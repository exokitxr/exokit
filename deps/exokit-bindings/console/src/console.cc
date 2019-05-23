#include <console.h>

namespace console {
  
NAN_METHOD(Log) {
  Local<Uint8Array> array = Local<Uint8Array>::Cast(info[0]);
  char *data = (char *)array->Buffer()->GetContents().Data() + array->ByteOffset();
  size_t length = array->ByteLength();
  fwrite(data, length, 1, stdout);
}

Local<Object> Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  Local<Object> result = Nan::New<Object>();
  
  Local<FunctionTemplate> logFnTemplate = Nan::New<FunctionTemplate>(Log);
  Local<Function> logFn = Nan::GetFunction(logFnTemplate).ToLocalChecked();
  result->Set(JS_STR("Log"), logFn);

  return scope.Escape(result);
}

};
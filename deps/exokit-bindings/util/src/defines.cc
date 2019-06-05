#include <defines.h>

Local<Array> pointerToArray(void *ptr) {
  uintptr_t n = (uintptr_t)ptr;
  Local<Array> result = Nan::New<Array>(2);
  result->Set(0, JS_NUM((uint32_t)(n >> 32)));
  result->Set(1, JS_NUM((uint32_t)(n & 0xFFFFFFFF)));
  return result;
}

uintptr_t arrayToPointer(Local<Array> array) {
  return ((uintptr_t)TO_UINT32(array->Get(0)) << 32) | (uintptr_t)TO_UINT32(array->Get(1));
}

uintptr_t arrayToPointer(Local<Uint32Array> array) {
  return ((uintptr_t)TO_UINT32(array->Get(0)) << 32) | (uintptr_t)TO_UINT32(array->Get(1));
}

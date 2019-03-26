#include <defines.h>

Local<Array> pointerToArray(void *ptr) {
  uintptr_t n = (uintptr_t)ptr;
  Local<Array> result = Nan::New<Array>(2);
  result->Set(0, JS_NUM((uint32_t)(n >> 32)));
  result->Set(1, JS_NUM((uint32_t)(n & 0xFFFFFFFF)));
  return result;
}

void *arrayToPointer(Local<Array> array) {
  uintptr_t n = ((uintptr_t)TO_UINT32(array->Get(0)) << 32) | (uintptr_t)TO_UINT32(array->Get(1));
  return (void *)n;
}

void *arrayToPointer(Local<Uint32Array> array) {
  uintptr_t n = ((uintptr_t)TO_UINT32(array->Get(0)) << 32) | (uintptr_t)TO_UINT32(array->Get(1));
  return (void *)n;
}

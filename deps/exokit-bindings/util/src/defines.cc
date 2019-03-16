#include <defines.h>

using namespace Nan;

Local<Array> pointerToArray(void *ptr) {
  uintptr_t n = (uintptr_t)ptr;
  Local<Array> result = Nan::New<Array>(2);
  result->Set(0, JS_NUM((uint32_t)(n >> 32)));
  result->Set(1, JS_NUM((uint32_t)(n & 0xFFFFFFFF)));
  return result;
}

void *arrayToPointer(Local<Array> array) {

  uintptr_t n0 = To<uint32_t>(Nan::Get(array, 0).ToLocalChecked()).FromJust();
  uintptr_t n1 = To<uint32_t>(Nan::Get(array, 1).ToLocalChecked()).FromJust();
  uintptr_t n = (n0 << 32) | n1;
  return (void *)n;
}

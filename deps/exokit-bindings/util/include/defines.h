#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <v8.h>
#include <nan.h>

using namespace v8;

#define JS_STR(...) Nan::New<v8::String>(__VA_ARGS__).ToLocalChecked()
#define JS_INT(val) Nan::New<v8::Integer>(val)
#define JS_NUM(val) Nan::New<v8::Number>(val)
#define JS_FLOAT(val) Nan::New<v8::Number>(val)
#define JS_BOOL(val) Nan::New<v8::Boolean>(val)
#define JS_OBJ(val) Nan::To<v8::Object>(val).ToLocalChecked()

#define TO_DOUBLE(x) (Nan::To<double>(x).FromJust())
#define TO_BOOL(x) (Nan::To<bool>(x).FromJust())
#define TO_UINT32(x) (Nan::To<unsigned int>(x).FromJust())
#define TO_INT32(x) (Nan::To<int>(x).FromJust())
#define TO_FLOAT(x) static_cast<float>((Nan::To<double>(x).FromJust()))

template <typename T>
class shared_ptr_release_deleter {
public:
  void operator() (T *ptr) {
    // nothing
  }
private:
};

Local<Array> pointerToArray(void *ptr);
void *arrayToPointer(Local<Array> array);
void *arrayToPointer(Local<Uint32Array> array);

template <typename T> struct V8TypedArrayTraits;
template<> struct V8TypedArrayTraits<Float32Array> { typedef float value_type; };
template<> struct V8TypedArrayTraits<Float64Array> { typedef double value_type; };
template<> struct V8TypedArrayTraits<Int32Array> { typedef int value_type; };
template<> struct V8TypedArrayTraits<Uint32Array> { typedef unsigned int value_type; };

template <typename T>
Local<T> createTypedArray(size_t size, const typename V8TypedArrayTraits<T>::value_type* data = NULL) {
  size_t byteLength = size * sizeof(typename V8TypedArrayTraits<T>::value_type);
  Local<ArrayBuffer> buffer = ArrayBuffer::New(Isolate::GetCurrent(), byteLength);
  Local<T> result = T::New(buffer, 0, size);
  if (data) {
    for (unsigned int i = 0; i < size; i++) {
      result->Set(i, Nan::New(data[i]));
    }
  }
  return result;
};

#endif

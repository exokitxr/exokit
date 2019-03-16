#pragma once

#define JS_FUNC(x) (Nan::GetFunction(x).ToLocalChecked())
#define JS_OBJ(x) (Nan::To<v8::Object>(x).ToLocalChecked())
#define JS_NUM(x) (Nan::To<double>(x).FromJust())
#define JS_BOOL(x) (Nan::To<bool>(x).FromJust())
#define JS_UINT32(x) (Nan::To<unsigned int>(x).FromJust())
#define JS_INT32(x) (Nan::To<int>(x).FromJust())
#define JS_ISOLATE() (v8::Isolate::GetCurrent())
#define JS_CONTEXT() (JS_ISOLATE()->GetCurrentContext())
#define JS__HAS(x, y) (((x)->Has(JS_CONTEXT(), (y))).FromJust())

#define EXO_ToString(x) (Nan::To<v8::String>(x).ToLocalChecked())

#define UINT32_TO_JS(x) (Nan::New(static_cast<uint32_t>(x)))
#define INT32_TO_JS(x) (Nan::New(static_cast<int32_t>(x)))
#define BOOL_TO_JS(x) ((x) ? Nan::True() : Nan::False())
#define DOUBLE_TO_JS(x) (Nan::New(static_cast<double>(x)))
#define FLOAT_TO_JS(x) (Nan::New(static_cast<float>(x)))

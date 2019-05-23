#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <thread>
#include <functional>

#include <v8.h>
#include <bindings.h>

#ifdef OPENVR
#include <openvr-bindings.h>
#endif
#ifdef ANDROID
#include <oculus-mobile.h>
#endif

#ifdef OCULUSVR
#include <oculus-bindings.h>
#endif

using namespace v8;

namespace exokit {

// NOTE: must already be in context
void callFunction(const char *funcname, const int argc, Local<Value> argv[]) {
  // init
  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> localContext = isolate->GetCurrentContext();
  Local<Object> global = localContext->Global();

  // get function
  Local<String> jsfunc_name = String::NewFromUtf8(isolate,funcname);
  Local<Value> jsfunc_val = global->Get(jsfunc_name);
  if (!jsfunc_val->IsFunction()) return;
  Local<Function> jsfunc = Local<Function>::Cast(jsfunc_val);

  // call function, 'this' points to global object
  TryCatch try_catch(Isolate::GetCurrent());
  Local<Value> result = jsfunc->Call(Isolate::GetCurrent()->GetCurrentContext(), global, argc, argv).ToLocalChecked();

  if (result.IsEmpty()) {
    Nan::Utf8String error(try_catch.Exception());
    Nan::Utf8String stacktrace(try_catch.StackTrace(localContext).ToLocalChecked());
    // LOGI("Error calling %s: %s:\n%s",funcname,*error,*stacktrace);
  } else {
    // LOGI("%s called",funcname);
  }
}

void Java_com_mafintosh_nodeonandroid_NodeService_onResize
() {
// (JNIEnv *env, jclass clas, jint width, jint height) {
	// LOGI("JNI onResize %d %d", width, height);

  {
    HandleScope handle_scope(Isolate::GetCurrent());

    unsigned int width = 1;
    unsigned int height = 1;

    Local<Number> js_width = v8::Integer::New(Isolate::GetCurrent(), width);
    Local<Number> js_height = v8::Integer::New(Isolate::GetCurrent(), height);

    Local<Value> argv[] = {js_width, js_height};
    callFunction("onResize", sizeof(argv)/sizeof(argv[0]), argv);
  }
}


void Java_com_mafintosh_nodeonandroid_NodeService_onNewFrame
() {
// (JNIEnv *env, jclass clas, jfloatArray headViewMatrix, jfloatArray headQuaternion, jfloatArray centerArray) {
  float headViewMatrixElements[] = {0};
  float headQuaternionElements[] = {0};
  float centerArrayElements[] = {0};

  {
    HandleScope handle_scope(Isolate::GetCurrent());

    Local<Float32Array> headMatrixFloat32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 16 * 4), 0, 16);
    for (int i = 0; i < 16; i++) {
      headMatrixFloat32Array->Set(i, Number::New(Isolate::GetCurrent(), headViewMatrixElements[i]));
    }
    Local<Float32Array> headQuaternionFloat32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 4 * 4), 0, 4);
    for (int i = 0; i < 4; i++) {
      headQuaternionFloat32Array->Set(i, Number::New(Isolate::GetCurrent(), headQuaternionElements[i]));
    }
    Local<Float32Array> centerFloat32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 3 * 4), 0, 3);
    for (int i = 0; i < 3; i++) {
      centerFloat32Array->Set(i, Number::New(Isolate::GetCurrent(), centerArrayElements[i]));
    }
    Local<Value> argv[] = {headMatrixFloat32Array, headQuaternionFloat32Array, centerFloat32Array};
    callFunction("onNewFrame", sizeof(argv)/sizeof(argv[0]), argv);
  }
}


void Java_com_mafintosh_nodeonandroid_NodeService_onDrawEye
() {
// (JNIEnv *env, jclass clasj, jfloatArray eyeViewMatrix, jfloatArray eyePerspectiveMatrix) {
  float eyeViewMatrixElements[] = {0};
  float eyePerspectiveMatrixElements[] = {0};

  {
    HandleScope handle_scope(Isolate::GetCurrent());

    Local<Float32Array> eyeViewMatrixFloat32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 16 * 4), 0, 16);
    for (int i = 0; i < 16; i++) {
      eyeViewMatrixFloat32Array->Set(i, Number::New(Isolate::GetCurrent(), eyeViewMatrixElements[i]));
    }
    Local<Float32Array> eyePerspectiveMatrixFloat32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 4 * 4), 0, 4);
    for (int i = 0; i < 4; i++) {
      eyePerspectiveMatrixFloat32Array->Set(i, Number::New(Isolate::GetCurrent(), eyePerspectiveMatrixElements[i]));
    }
    Local<Value> argv[] = {eyeViewMatrixFloat32Array, eyePerspectiveMatrixFloat32Array};
    callFunction("onDrawEye", sizeof(argv)/sizeof(argv[0]), argv);
  }
}


void Java_com_mafintosh_nodeonandroid_NodeService_onDrawFrame
() {
// (JNIEnv *env, jclass clas, jfloatArray viewMatrix, jfloatArray projectionMatrix, jfloatArray centerArray) {
  float viewMatrixElements[] = {0};
  float projectionMatrixElements[] = {0};
  float centerArrayElements[] = {0};

  {
    HandleScope handle_scope(Isolate::GetCurrent());

    Local<Float32Array> viewFloat32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 16 * 4), 0, 16);
    for (int i = 0; i < 16; i++) {
      viewFloat32Array->Set(i, Number::New(Isolate::GetCurrent(), viewMatrixElements[i]));
    }
    Local<Float32Array> projectionFloat32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 16 * 4), 0, 16);
    for (int i = 0; i < 16; i++) {
      projectionFloat32Array->Set(i, Number::New(Isolate::GetCurrent(), projectionMatrixElements[i]));
    }
    Local<Float32Array> centerFloat32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 3 * 4), 0, 3);
    for (int i = 0; i < 3; i++) {
      centerFloat32Array->Set(i, Number::New(Isolate::GetCurrent(), centerArrayElements[i]));
    }
    Local<Value> argv[] = {viewFloat32Array, projectionFloat32Array, centerFloat32Array};
    callFunction("onDrawFrame", sizeof(argv)/sizeof(argv[0]), argv);
  }
}

void InitExports(Local<Object> exports) {
  std::pair<Local<Value>, Local<FunctionTemplate>> glResult = makeGl();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeGl"), glResult.first);

  std::pair<Local<Value>, Local<FunctionTemplate>> gl2Result = makeGl2(glResult.second);
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeGl2"), gl2Result.first);

  Local<Value> image = makeImage();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeImage"), image);

  Local<Value> imageData = makeImageData();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeImageData"), imageData);

  Local<Value> imageBitmap = makeImageBitmap();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeImageBitmap"), imageBitmap);

  Local<Value> path2d = makePath2D();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativePath2D"), path2d);

  Local<Value> canvasGradient = makeCanvasGradient();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeCanvasGradient"), canvasGradient);

  Local<Value> canvasPattern = makeCanvasPattern();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeCanvasPattern"), canvasPattern);

  Local<Value> canvas = makeCanvasRenderingContext2D(imageData, canvasGradient, canvasPattern);
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeCanvasRenderingContext2D"), canvas);

  Local<Value> audio = makeAudio();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeAudio"), audio);

  Local<Value> video = makeVideo(imageData);
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeVideo"), video);

#if !defined(ANDROID)
  Local<Value> browser = makeBrowser();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeBrowser"), browser);
#endif

  Local<Value> rtc = makeRtc();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeRtc"), rtc);

  /* Local<Value> glfw = makeGlfw();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeGlfw"), glfw); */

  Local<Value> window = makeWindow();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeWindow"), window);

#ifdef OPENVR
  Local<Value> vr = makeOpenVR();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeOpenVR"), vr);
#endif

#ifdef OCULUSVR
  Local<Value> oculusVR = makeOculusVR();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeOculusVR"), oculusVR);
#endif

#if LEAPMOTION
  Local<Value> lm = makeLm();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeLm"), lm);
#endif

#ifdef ANDROID
  Local<Value> oculusMobileVr = makeOculusMobileVr();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeOculusMobileVr"), oculusMobileVr);
#endif

#if defined(LUMIN)
  Local<Value> ml = makeMl();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeMl"), ml);
#endif

#if defined(ANDROID)
#define NATIVE_PLATFORM "android"
#elif defined(LUMIN)
#define NATIVE_PLATFORM "lumin"
#else
#define NATIVE_PLATFORM ""
#endif
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativePlatform"), JS_STR(NATIVE_PLATFORM));

  /* uintptr_t initFunctionAddress = (uintptr_t)InitExports;
  Local<Array> initFunctionAddressArray = Nan::New<Array>(2);
  initFunctionAddressArray->Set(0, Nan::New<Integer>((uint32_t)(initFunctionAddress >> 32)));
  initFunctionAddressArray->Set(1, Nan::New<Integer>((uint32_t)(initFunctionAddress & 0xFFFFFFFF)));
  exports->Set(JS_STR("initFunctionAddress"), initFunctionAddressArray); */
}

void Init(Local<Object> exports) {
  InitExports(exports);
}

}

#if !defined(ANDROID) && !defined(LUMIN)
NODE_MODULE_INIT(/* exports, module, context */) {
  exokit::Init(exports);
}
#else
extern "C" {
  void node_register_module_exokit(Local<Object> exports, Local<Value> module, Local<Context> context) {
    exokit::Init(exports);
  }
}
#endif

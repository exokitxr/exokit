#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <thread>
#include <functional>

#include <bindings.h>
#include <v8.h>

#ifdef OPENVR
#include <openvr-bindings.h>
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
  Local<Value> jsfunc_val = Nan::Get(global, Nan::New<String>(funcname).ToLocalChecked()).ToLocalChecked();
  if (!jsfunc_val->IsFunction()) return;
  Local<Function> jsfunc = Local<Function>::Cast(jsfunc_val);

  // call function, 'this' points to global object
  TryCatch try_catch(Isolate::GetCurrent());
  Local<Value> result = Nan::Call(jsfunc, global, argc, argv).ToLocalChecked();

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

    Local<Value> argv[] = {Nan::New<Integer>(width), Nan::New<Integer>(height)};
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
  exports->Set(Nan::New("nativeGl").ToLocalChecked(), glResult.first);

  std::pair<Local<Value>, Local<FunctionTemplate>> gl2Result = makeGl2(glResult.second);
  exports->Set(Nan::New("nativeGl2").ToLocalChecked(), gl2Result.first);

  Local<Value> image = makeImage();
  exports->Set(Nan::New("nativeImage").ToLocalChecked(), image);

  Local<Value> imageData = makeImageData();
  exports->Set(Nan::New("nativeImageData").ToLocalChecked(), imageData);

  Local<Value> imageBitmap = makeImageBitmap();
  exports->Set(Nan::New("nativeImageBitmap").ToLocalChecked(), imageBitmap);

  Local<Value> path2d = makePath2D();
  exports->Set(Nan::New("nativePath2D").ToLocalChecked(), path2d);

  Local<Value> canvasGradient = makeCanvasGradient();
  exports->Set(Nan::New("nativeCanvasGradient").ToLocalChecked(), canvasGradient);

  Local<Value> canvasPattern = makeCanvasPattern();
  exports->Set(Nan::New("nativeCanvasPattern").ToLocalChecked(), canvasPattern);

  Local<Value> canvas = makeCanvasRenderingContext2D(imageData, canvasGradient, canvasPattern);
  exports->Set(Nan::New("nativeCanvasRenderingContext2D").ToLocalChecked(), canvas);

  Local<Value> audio = makeAudio();
  exports->Set(Nan::New("nativeAudio").ToLocalChecked(), audio);

  Local<Value> video = makeVideo(imageData);
  exports->Set(Nan::New("nativeVideo").ToLocalChecked(), video);
  
  Local<Value> browser = makeBrowser();
  exports->Set(Nan::New("nativeBrowser").ToLocalChecked(), browser);

  Local<Value> rtc = makeRtc();
  exports->Set(Nan::New("nativeRtc").ToLocalChecked(), rtc);

  /* Local<Value> glfw = makeGlfw();
  exports->Set(Nan::New("nativeGlfw").ToLocalChecked(), glfw); */

  Local<Value> window = makeWindow();
  exports->Set(Nan::New("nativeWindow").ToLocalChecked(), window);

#ifdef OPENVR
  Local<Value> vr = makeVr();
  exports->Set(Nan::New("nativeVr").ToLocalChecked(), vr);
#endif

#if LEAPMOTION
  Local<Value> lm = makeLm();
  exports->Set(Nan::New("nativeLm").ToLocalChecked(), lm);
#endif

#if defined(LUMIN)
  Local<Value> ml = makeMl();
  exports->Set(Nan::New("nativeMl").ToLocalChecked(), ml);
#endif

#ifndef LUMIN
#define NATIVE_ANALYTICS true
#else
#define NATIVE_ANALYTICS false
#endif
  exports->Set(Nan::New("nativeAnalytics").ToLocalChecked(), JS_BOOL(NATIVE_ANALYTICS));

  uintptr_t initFunctionAddress = (uintptr_t)InitExports;
  Local<Array> initFunctionAddressArray = Nan::New<Array>(2);
  initFunctionAddressArray->Set(0, Nan::New<Integer>((uint32_t)(initFunctionAddress >> 32)));
  initFunctionAddressArray->Set(1, Nan::New<Integer>((uint32_t)(initFunctionAddress & 0xFFFFFFFF)));
  exports->Set(Nan::New("initFunctionAddress").ToLocalChecked(), initFunctionAddressArray);
}

void Init(Local<Object> exports) {
  InitExports(exports);
}

}

#ifndef LUMIN
NODE_MODULE(NODE_GYP_MODULE_NAME, exokit::Init)
#else
extern "C" {
  void node_register_module_exokit(Local<Object> exports, Local<Value> module, Local<Context> context) {
    exokit::Init(exports);
  }
}
#endif

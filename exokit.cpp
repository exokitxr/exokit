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

#ifdef LUMIN
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

  Local<Value> console = makeConsole();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeConsole"), console);

  Local<Value> cache = makeCache();
  exports->Set(v8::String::NewFromUtf8(Isolate::GetCurrent(), "nativeCache"), cache);

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

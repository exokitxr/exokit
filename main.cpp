// (c) 2014 Boris van Schooten
//BEGIN_INCLUDE(all)
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <sstream>
#include <thread>
#include <functional>

#include <jni.h>
#include <errno.h>

// #include <EGL/egl.h>
// at least some defs from gl1 are needed
// #include <GLES/gl.h>
// #include <GLES2/gl2.h>

#include <v8.h>
#include <bindings.h>

using namespace v8;

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
  TryCatch try_catch;
  Local<Value> result = jsfunc->Call(global, argc, argv);

  if (result.IsEmpty()) {
    String::Utf8Value error(try_catch.Exception());
    String::Utf8Value stacktrace(try_catch.StackTrace());
    LOGI("Error calling %s: %s:\n%s",funcname,*error,*stacktrace);
  } else {
    //LOGI("%s called",funcname);
  }
}

void redirectStdioToLog() {
  setvbuf(stdout, 0, _IOLBF, 0);
  setvbuf(stderr, 0, _IONBF, 0);

  int pfd[2];
  pipe(pfd);
  dup2(pfd[1], 1);
  dup2(pfd[1], 2);

  std::thread([](int pfd0) {
    char buf[1024];
    std::size_t nBytes = 0;
    while ((nBytes = read(pfd0, buf, sizeof buf - 1)) > 0) {
      if (buf[nBytes - 1] == '\n') --nBytes;
      buf[nBytes] = 0;
      LOGI("%s", buf);
    }
  }, pfd[0]).detach();
}


#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_onResize
(JNIEnv *env, jclass clas, jint width, jint height) {
	LOGI("JNI onResize %d %d", width, height);

  {
    HandleScope handle_scope(Isolate::GetCurrent());

    Handle<Number> js_width = v8::Integer::New(Isolate::GetCurrent(), width);
    Handle<Number> js_height = v8::Integer::New(Isolate::GetCurrent(), height);

    Local<Value> argv[] = {js_width, js_height};
    callFunction("onResize", sizeof(argv)/sizeof(argv[0]), argv);
  }
}


JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_onNewFrame
(JNIEnv *env, jclass clas, jfloatArray headViewMatrix, jfloatArray headQuaternion, jfloatArray centerArray) {
  jfloat *headViewMatrixElements = env->GetFloatArrayElements(headViewMatrix, 0);
  jfloat *headQuaternionElements = env->GetFloatArrayElements(headQuaternion, 0);
  jfloat *centerArrayElements = env->GetFloatArrayElements(centerArray, 0);

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

  env->ReleaseFloatArrayElements(headViewMatrix, headViewMatrixElements, 0);
  env->ReleaseFloatArrayElements(headQuaternion, headQuaternionElements, 0);
  env->ReleaseFloatArrayElements(centerArray, centerArrayElements, 0);
}


JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_onDrawEye
(JNIEnv *env, jclass clasj, jfloatArray eyeViewMatrix, jfloatArray eyePerspectiveMatrix) {
  jfloat *eyeViewMatrixElements = env->GetFloatArrayElements(eyeViewMatrix, 0);
  jfloat *eyePerspectiveMatrixElements = env->GetFloatArrayElements(eyePerspectiveMatrix, 0);

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

  env->ReleaseFloatArrayElements(eyeViewMatrix, eyeViewMatrixElements, 0);
  env->ReleaseFloatArrayElements(eyePerspectiveMatrix, eyePerspectiveMatrixElements, 0);
}


JNIEXPORT void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_onDrawFrame
(JNIEnv *env, jclass clas, jfloatArray viewMatrix, jfloatArray projectionMatrix, jfloatArray centerArray) {
  jfloat *viewMatrixElements = env->GetFloatArrayElements(viewMatrix, 0);
  jfloat *projectionMatrixElements = env->GetFloatArrayElements(projectionMatrix, 0);
  jfloat *centerArrayElements = env->GetFloatArrayElements(centerArray, 0);

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

  env->ReleaseFloatArrayElements(viewMatrix, viewMatrixElements, 0);
  env->ReleaseFloatArrayElements(projectionMatrix, projectionMatrixElements, 0);
  env->ReleaseFloatArrayElements(centerArray, centerArrayElements, 0);
}

std::function<void (node::NodeService *nodeService)> nodeServiceInitFunction;
void JNICALL Java_com_mafintosh_nodeonandroid_NodeService_start
(JNIEnv *env, jobject thiz, jstring binPath, jstring jsPath, jstring libpath, jobject assetManager, jstring url, jstring vrMode, jint vrTexture) {
  redirectStdioToLog();

  /* AAssetManager *aAssetManager = AAssetManager_fromJava(env, assetManager);
  canvas::AndroidContextFactory *canvasContextFactory = new canvas::AndroidContextFactory(aAssetManager, 1); */
  canvas::AndroidContextFactory::initialize(env, assetManager);
  canvas::AndroidContextFactory *canvasContextFactory = new canvas::AndroidContextFactory(nullptr, 1);

  const char *binPathString = env->GetStringUTFChars(binPath, NULL);
  const char *jsPathString = env->GetStringUTFChars(jsPath, NULL);
  const char *libPathString = env->GetStringUTFChars(libpath, NULL);
  const char *urlString = env->GetStringUTFChars(url, NULL);
  const char *vrModeString = env->GetStringUTFChars(vrMode, NULL);
  std::stringstream vrTextureStringStream;
  vrTextureStringStream << vrTexture;
  const char *vrTextureString = vrTextureStringStream.str().c_str();

  char argsString[4096];
  int i = 0;

  char *binPathArg = argsString + i;
  strncpy(binPathArg, binPathString, sizeof(argsString) - i);
  i += strlen(binPathString) + 1;

  char *jsPathArg = argsString + i;
  strncpy(jsPathArg, jsPathString, sizeof(argsString) - i);
  i += strlen(jsPathString) + 1;

  char *libPathArg = argsString + i;
  strncpy(libPathArg, libPathString, sizeof(argsString) - i);
  i += strlen(libPathString) + 1;

  char *urlArg = argsString + i;
  strncpy(urlArg, urlString, sizeof(argsString) - i);
  i += strlen(urlString) + 1;

  char *vrModeArg = argsString + i;
  strncpy(vrModeArg, vrModeString, sizeof(argsString) - i);
  i += strlen(vrModeString) + 1;

  char *vrTextureArg = argsString + i;
  strncpy(vrTextureArg, vrTextureString, sizeof(argsString) - i);
  i += strlen(vrTextureString) + 1;

  char *args[] = {binPathArg, jsPathArg, libPathArg, urlArg, vrModeArg, vrTextureArg};

  nodeServiceInitFunction = [&](node::NodeService *service) {
    Isolate *isolate = Isolate::GetCurrent();
    Local<Object> global = isolate->GetCurrentContext()->Global();

    Local<Value> gl = makeGl(service);
    global->Set(v8::String::NewFromUtf8(isolate, "nativeGl"), gl);

    Local<Value> image = makeImage(service, canvasContextFactory);
    global->Set(v8::String::NewFromUtf8(isolate, "nativeImage"), image);

    Local<Value> imageData = makeImageData(service);
    global->Set(v8::String::NewFromUtf8(isolate, "nativeImageData"), imageData);

    Local<Value> imageBitmap = makeImageBitmap(service);
    global->Set(v8::String::NewFromUtf8(isolate, "nativeImageBitmap"), imageBitmap);

    Local<Value> canvas = makeCanvasRenderingContext2D(service, canvasContextFactory, imageData);
    global->Set(v8::String::NewFromUtf8(isolate, "nativeCanvasRenderingContext2D"), canvas);

    Local<Value> path2d = makePath2D(service);
    global->Set(v8::String::NewFromUtf8(isolate, "nativePath2D"), path2d);
  };
  service = new node::NodeService(sizeof(args)/sizeof(args[0]), args, [](node::NodeService *service) {
    nodeServiceInitFunction(service);
  });

  std::function<void (node::NodeService *nodeService)> nopFunction;
  nodeServiceInitFunction = nopFunction;
}


#ifdef __cplusplus
}
#endif

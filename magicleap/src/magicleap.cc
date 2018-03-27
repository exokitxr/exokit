#ifdef MAGICLEAP

#include <magicleap.h>
#include <uv.h>

using namespace v8;
using namespace std;

namespace ml {

const char application_name[] = "com.magicleap.simpleglapp";
application_context_t application_context;
MLLifecycleCallbacks lifecycle_callbacks = {};
MLLifecycleErrorCode lifecycle_status;
std::thread *initThread;
uv_async_t async;
bool initialized = false;
Nan::Persistent<Function> initCb;

bool isPresent() {
  return initialized && lifecycle_status == MLLifecycleErrorCode_Success;
}

void asyncCb(uv_async_t *handle) {
  Nan::HandleScope scope;

  Local<Function> initCbFn = Nan::New(initCb);
  Local<Value> args[] = {
    JS_BOOL(isPresent()),
  };
  initCbFn->Call(Nan::Null(), sizeof(args)/sizeof(args[0]), args);
}

static void onStop(void* application_context) {
  ((struct application_context_t*)application_context)->dummy_value = 0;
  ML_LOG(Info, "%s: On stop called.", application_name);
}

static void onPause(void* application_context) {
  ((struct application_context_t*)application_context)->dummy_value = 1;
  ML_LOG(Info, "%s: On pause called.", application_name);
}

static void onResume(void* application_context) {
  ((struct application_context_t*)application_context)->dummy_value = 2;
  ML_LOG(Info, "%s: On resume called.", application_name);
}

MLContext::MLContext() {}

MLContext::~MLContext() {}

Handle<Object> MLContext::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("MLContext"));
  Nan::SetMethod(ctor, "IsPresent", IsPresent);
  Nan::SetMethod(ctor, "OnPresentChange", OnPresentChange);

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "Init", Init);
  Nan::SetMethod(proto, "WaitGetPoses", WaitGetPoses);
  Nan::SetMethod(proto, "SubmitFrame", SubmitFrame);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_METHOD(MLContext::New) {
  Nan::HandleScope scope;

  Local<Object> mlContextObj = info.This();
  MLContext *mlContext = new MLContext();
  mlContext->Wrap(mlContextObj);

  if (!initThread) {
    uv_async_init(uv_default_loop(), &async, asyncCb);

    initThread = new std::thread(LifecycleInit);

    std::atexit([]() {
      initThread->join();
      delete initThread;
      initThread = nullptr;

      quick_exit(0);
    });
  }

  info.GetReturnValue().Set(mlContextObj);
}

NAN_METHOD(MLContext::Init) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));

  if (lifecycle_status != MLLifecycleErrorCode_Success) {
    ML_LOG(Error, "%s: Failed to initialize lifecycle.", application_name);
    info.GetReturnValue().Set(JS_BOOL(false));
    return;
  }

  // initialize perception system
  MLPerceptionSettings perception_settings;

  bool perception_init_status = MLPerceptionInitSettings(&perception_settings);
  if (perception_init_status != true) {
    ML_LOG(Error, "%s: Failed to initialize perception.", application_name);
    info.GetReturnValue().Set(JS_BOOL(false));
    return;
  }

  bool perception_startup_status = MLPerceptionStartup(&perception_settings);
  if (perception_startup_status != true) {
    ML_LOG(Error, "%s: Failed to startup perception.", application_name);
    info.GetReturnValue().Set(JS_BOOL(false));
    return;
  }

  MLGraphicsOptions graphics_options = { MLGraphicsFlags_NoDepth, MLSurfaceFormat_RGBA8UNorm, MLSurfaceFormat_D32Float };
  MLHandle opengl_context = reinterpret_cast<MLHandle>(window);
  mlContext->graphics_client = ML_INVALID_HANDLE;
  MLStatus graphics_create_status = MLStatus_OK;
  MLGraphicsCreateClientGL(&graphics_options, opengl_context, &mlContext->graphics_client, &graphics_create_status);

  // Now that graphics is connected, the app is ready to go
  if (!MLLifecycleSetReadyIndication(&lifecycle_status)) {
    ML_LOG(Error, "%s: Failed to indicate lifecycle ready.", application_name);
    info.GetReturnValue().Set(JS_BOOL(false));
    return;
  }

  mlContext->head_tracker = MLHeadTrackingCreate();
  if (MLHandleIsValid(mlContext->head_tracker)) {
    MLHeadTrackingGetStaticData(mlContext->head_tracker, &mlContext->head_static_data);
  } else {
    ML_LOG(Error, "%s: Failed to create head tracker.", application_name);
  }

  ML_LOG(Info, "%s: Start loop.", application_name);

  GLuint framebuffers[2];
  glGenFramebuffers(2, framebuffers);
  mlContext->framebuffer_id = framebuffers[0];
  mlContext->temp_framebuffer_id = framebuffers[1];

  info.GetReturnValue().Set(JS_BOOL(true));
}

NAN_METHOD(MLContext::WaitGetPoses) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  if (info[0]->IsUint32Array() && info[1]->IsFloat32Array() && info[2]->IsFloat32Array() && info[3]->IsUint32Array()) {
    if (application_context.dummy_value) {
      Local<Uint32Array> framebufferArray = Local<Uint32Array>::Cast(info[0]);
      Local<Float32Array> transformArray = Local<Float32Array>::Cast(info[1]);
      Local<Float32Array> projectionArray = Local<Float32Array>::Cast(info[2]);
      Local<Uint32Array> viewportArray = Local<Uint32Array>::Cast(info[3]);

      MLStatus out_status;
      MLGraphicsFrameParams frame_params;
      if (!MLGraphicsInitFrameParams(&frame_params, &out_status)) {
        ML_LOG(Error, "MLGraphicsBeginFrame complained: %d", out_status);
      }
      frame_params.surface_scale = 1.0f;
      frame_params.projection_type = MLGraphicsProjectionType_ReversedInfiniteZ;
      frame_params.near_clip = 1.0f;
      frame_params.focus_distance = 1.0f;

      MLGraphicsBeginFrame(mlContext->graphics_client, &frame_params, &mlContext->frame_handle, &mlContext->virtual_camera_array, &out_status);
      if (out_status != MLStatus_OK) {
        ML_LOG(Error, "MLGraphicsBeginFrame complained: %d", out_status);
      }

      framebufferArray->Set(0, JS_INT((unsigned int)mlContext->virtual_camera_array.color_id));
      framebufferArray->Set(1, JS_INT((unsigned int)mlContext->virtual_camera_array.depth_id));

      for (int i = 0; i < 2; i++) {
        const MLGraphicsVirtualCameraInfo &cameraInfo = mlContext->virtual_camera_array.virtual_cameras[i];
        const MLTransform &transform = cameraInfo.transform;
        transformArray->Set(i*7 + 0, JS_NUM(transform.position.x));
        transformArray->Set(i*7 + 1, JS_NUM(transform.position.y));
        transformArray->Set(i*7 + 2, JS_NUM(transform.position.z));
        transformArray->Set(i*7 + 3, JS_NUM(transform.rotation.x));
        transformArray->Set(i*7 + 4, JS_NUM(transform.rotation.y));
        transformArray->Set(i*7 + 5, JS_NUM(transform.rotation.z));
        transformArray->Set(i*7 + 6, JS_NUM(transform.rotation.w));

        const MLMat4f &projection = cameraInfo.projection;
        for (int j = 0; j < 16; j++) {
          projectionArray->Set(i*16 + j, JS_NUM(projection.matrix_colmajor[j]));
        }
      }

      const MLRectf& viewport = mlContext->virtual_camera_array.viewport;
      viewportArray->Set(0, JS_INT((int)viewport.x));
      viewportArray->Set(1, JS_INT((int)viewport.y));
      viewportArray->Set(2, JS_INT((unsigned int)viewport.w));
      viewportArray->Set(3, JS_INT((unsigned int)viewport.h));
    } else {
      Nan::ThrowError("MLContext::WaitGetPoses called for dead app");
    }
  } else {
    Nan::ThrowError("MLContext::WaitGetPoses: invalid arguments");
  }
}

NAN_METHOD(MLContext::SubmitFrame) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  GLuint src_framebuffer_id = info[0]->Uint32Value();
  unsigned int width = info[1]->Uint32Value();
  unsigned int height = info[2]->Uint32Value();

  const MLRectf &viewport = mlContext->virtual_camera_array.viewport;

  MLStatus out_status;
  for (int camera = 0; camera < 2; ++camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, mlContext->framebuffer_id);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mlContext->virtual_camera_array.color_id, 0, camera);
    // glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mlContext->virtual_camera_array.depth_id, 0, camera);

    glBindFramebuffer(GL_FRAMEBUFFER, mlContext->temp_framebuffer_id);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mlContext->virtual_camera_array.color_id, 0);
    // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mlContext->virtual_camera_array.depth_id, 0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, src_framebuffer_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->temp_framebuffer_id);

    glBlitFramebuffer(0, 0,
      width, height,
      viewport.x, viewport.y,
      viewport.w * 2, viewport.h,
      GL_COLOR_BUFFER_BIT,
      GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    MLGraphicsSignalSyncObjectGL(mlContext->graphics_client, mlContext->virtual_camera_array.virtual_cameras[camera].sync_object, &out_status);
    if (out_status != MLStatus_OK) {
      ML_LOG(Error, "MLGraphicsSignalSyncObjectGL complained: %d", out_status);
    }
  }

  MLGraphicsEndFrame(mlContext->graphics_client, mlContext->frame_handle, &out_status);
  if (out_status != MLStatus_OK) {
    ML_LOG(Error, "MLGraphicsEndFrame complained: %d", out_status);
  }
}

NAN_METHOD(MLContext::IsPresent) {
  info.GetReturnValue().Set(JS_BOOL(isPresent()));
}

NAN_METHOD(MLContext::OnPresentChange) {
  if (info[0]->IsFunction()) {
    Local<Function> initCbFn = Local<Function>::Cast(info[0]);
    initCb.Reset(initCbFn);
  } else {
    Nan::ThrowError("not implemented");
  }
}

void MLContext::LifecycleInit() {
  application_context.dummy_value = 2;

  lifecycle_callbacks.on_stop = onStop;
  lifecycle_callbacks.on_pause = onPause;
  lifecycle_callbacks.on_resume = onResume;

  lifecycle_status = MLLifecycleInit(&lifecycle_callbacks, (void*)&application_context);

  initialized = true;

  uv_async_send(&async);
}

}

Handle<Object> makeMl() {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  return scope.Escape(ml::MLContext::Initialize(isolate));
}

#endif
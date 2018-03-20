#include <magicleap.h>

using namespace v8;
using namespace std;

namespace ml {

const char application_name[] = "com.magicleap.simpleglapp";

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

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "waitGetPoses", WaitGetPoses);
  Nan::SetMethod(proto, "submitFrame", SubmitFrame);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_METHOD(MLContext::New) {
  Nan::HandleScope scope;

  Local<Object> mlContextObj = info.This();
  MLContext *mlContext = new MLContext();
  mlContext->Wrap(mlContextObj);

  info.GetReturnValue().Set(mlContextObj);
}

NAN_METHOD(MLContext::Init) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  // let system know our app has started
  MLLifecycleCallbacks lifecycle_callbacks = {};
  lifecycle_callbacks.on_stop = onStop;
  lifecycle_callbacks.on_pause = onPause;
  lifecycle_callbacks.on_resume = onResume;

  mlContext->application_context.dummy_value = 2;

  MLLifecycleErrorCode lifecycle_status = MLLifecycleInit(&lifecycle_callbacks, (void*)&mlContext->application_context);
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

  MLGraphicsOptions graphics_options = { 0, MLSurfaceFormat_RGBA8UNorm, MLSurfaceFormat_D32Float };
  MLHandle opengl_context = reinterpret_cast<MLHandle>(glfwGetCurrentContext());
  mlContext->graphics_client = ML_INVALID_HANDLE;
  MLStatus graphics_create_status = MLStatus_OK;
  MLGraphicsCreateClientGL(&graphics_options, opengl_context, &mlContext->graphics_client, &graphics_create_status);

  // Now that graphics is connected, the app is ready to go
  if (!MLLifecycleSetReadyIndication(&lifecycle_status)) {
    ML_LOG(Error, "%s: Failed to indicate lifecycle ready.", application_name);
    info.GetReturnValue().Set(JS_BOOL(false));
    return;
  }

  MLHandle head_tracker = MLHeadTrackingCreate();
  MLHeadTrackingStaticData head_static_data;
  if (MLHandleIsValid(head_tracker)) {
    MLHeadTrackingGetStaticData(head_tracker, &head_static_data);
  } else {
    ML_LOG(Error, "%s: Failed to create head tracker.", application_name);
  }

  ML_LOG(Info, "%s: Start loop.", application_name);

  info.GetReturnValue().Set(JS_BOOL(true));
}

NAN_METHOD(MLContext::WaitGetPoses) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  if (info[0]->IsUint32Array() && info[1]->IsUint32Array()) {
    if (mlContext->application_context.dummy_value) {
      Local<Uint32Array> framebuffersArray = Local<Uint32Array>::Cast(info[0]);
      Local<Uint32Array> viewportArray = Local<Uint32Array>::Cast(info[1]);

      MLGraphicsFrameParams frame_params;

      MLStatus out_status;
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

      framebuffersArray->Set(0, JS_INT((unsigned int)mlContext->virtual_camera_array.color_id));
      framebuffersArray->Set(1, JS_INT((unsigned int)mlContext->virtual_camera_array.depth_id));

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

  for (int camera = 0; camera < 2; ++camera) {
    MLStatus out_status;
    MLGraphicsSignalSyncObjectGL(mlContext->graphics_client, mlContext->virtual_camera_array.virtual_cameras[camera].sync_object, &out_status);
    if (out_status != MLStatus_OK) {
      ML_LOG(Error, "MLGraphicsSignalSyncObjectGL complained: %d", out_status);
    }
  }

  MLStatus out_status;
  MLGraphicsEndFrame(mlContext->graphics_client, mlContext->frame_handle, &out_status);
  if (out_status != MLStatus_OK) {
    ML_LOG(Error, "MLGraphicsEndFrame complained: %d", out_status);
  }
}

NAN_METHOD(MLContext::Update) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  // auto start = std::chrono::steady_clock::now();

  while (mlContext->application_context.dummy_value) {
    MLGraphicsFrameParams frame_params;

    MLStatus out_status;
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

    // auto msRuntime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    // auto factor = labs(msRuntime % 2000 - 1000) / 1000.0;

    for (int camera = 0; camera < 2; ++camera) {
      /* glBindFramebuffer(GL_FRAMEBUFFER, graphics_context.framebuffer_id);
      glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mlContext->virtual_camera_array.color_id, 0, camera);
      glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mlContext->virtual_camera_array.depth_id, 0, camera); */

      const MLRectf& viewport = mlContext->virtual_camera_array.viewport;
      /* glViewport((GLint)viewport.x, (GLint)viewport.y,
                 (GLsizei)viewport.w, (GLsizei)viewport.h);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      if (camera == 0) {
        glClearColor(1.0 - factor, 0.0, 0.0, 0.0);
      } else {
        glClearColor(0.0, 0.0, factor, 0.0);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0); */
      MLGraphicsSignalSyncObjectGL(mlContext->graphics_client, mlContext->virtual_camera_array.virtual_cameras[camera].sync_object, &out_status);
    }
    MLGraphicsEndFrame(mlContext->graphics_client, mlContext->frame_handle, &out_status);
    if (out_status != MLStatus_OK) {
      ML_LOG(Error, "MLGraphicsEndFrame complained: %d", out_status);
    }

    // graphics_context.swapBuffers();
  }
}

}

Handle<Object> makeMl() {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  return scope.Escape(ml::MLContext::Initialize(isolate));
}
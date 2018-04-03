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

void makePlanesQueryer(MLHandle &planesHandle) {
  planesHandle = MLPlanesCreate();
  if (!MLHandleIsValid(planesHandle)) {
    ML_LOG(Error, "%s: Failed to create planes handle.", application_name);
  }
}
void beginPlanesQuery(MLHandle &planesHandle, MLHandle &planesQueryHandle, MLPlanesQueryFlags flags) {
  if (!MLHandleIsValid(planesQueryHandle)) {
    MLPlanesQuery query;
    query.flags = flags;
    query.bounds_center.x = 0;
    query.bounds_center.y = 0;
    query.bounds_center.z = 0;
    query.bounds_rotation.x = 0;
    query.bounds_rotation.y = 0;
    query.bounds_rotation.z = 0;
    query.bounds_rotation.w = 1;
    query.bounds_extents.x = 10;
    query.bounds_extents.y = 10;
    query.bounds_extents.z = 10;
    query.min_hole_length = 0.5;
    query.min_plane_area = 0.25;
    query.max_results = MAX_NUM_PLANES;

    planesQueryHandle = MLPlanesQueryBegin(planesHandle, &query);
    if (!MLHandleIsValid(planesQueryHandle)) {
      ML_LOG(Error, "%s: Failed to query planes.", application_name);
    }
  }
}
void endPlanesQuery(MLHandle &planesHandle, MLHandle &planesQueryHandle, MLPlane *planes, uint32_t *numPlanes) {
  if (MLHandleIsValid(planesQueryHandle)) {
    MLPlanesQueryResult planesQueryResult = MLPlanesQueryGetResults(planesHandle, planesQueryHandle, planes, numPlanes);
    if (planesQueryResult == MLPlanesQueryResult_Success) {
      planesQueryHandle = ML_INVALID_HANDLE;
    } else if (planesQueryResult == MLPlanesQueryResult_Failure) {
      planesQueryHandle = ML_INVALID_HANDLE;

      ML_LOG(Error, "MLPlanesQueryGetResults failed: %d %d %d %d %d", planesQueryResult, MLPlanesQueryResult_Success, MLPlanesQueryResult_Failure, MLPlanesQueryResult_Pending, MLPlanesQueryResult_Ensure32Bits);
    } else if (planesQueryResult == MLPlanesQueryResult_Pending) {
      // nothing, we wait
    } else {
      ML_LOG(Error, "MLPlanesQueryGetResults complained: %d %d %d %d %d", planesQueryResult, MLPlanesQueryResult_Success, MLPlanesQueryResult_Failure, MLPlanesQueryResult_Pending, MLPlanesQueryResult_Ensure32Bits);
    }
  }
}
void readPlanesQuery(MLPlane *planes, uint32_t numPlanes, int planeType, Local<Float32Array> &planesArray, uint32_t *planesIndex) {
  for (int i = 0; i < numPlanes; i++) {
    const MLPlane &plane = planes[i];
    uint32_t baseIndex = (*planesIndex) * PLANE_ENTRY_SIZE;
    planesArray->Set(baseIndex + 0, JS_NUM(plane.position.x));
    planesArray->Set(baseIndex + 1, JS_NUM(plane.position.y));
    planesArray->Set(baseIndex + 2, JS_NUM(plane.position.z));
    planesArray->Set(baseIndex + 3, JS_NUM(plane.rotation.x));
    planesArray->Set(baseIndex + 4, JS_NUM(plane.rotation.y));
    planesArray->Set(baseIndex + 5, JS_NUM(plane.rotation.z));
    planesArray->Set(baseIndex + 6, JS_NUM(plane.rotation.w));
    planesArray->Set(baseIndex + 7, JS_NUM(plane.width));
    planesArray->Set(baseIndex + 8, JS_NUM(plane.height));
    planesArray->Set(baseIndex + 9, JS_INT(planeType));

   (*planesIndex)++;
  }
}
int gestureCategoryToIndex(MLGestureStaticHandState gesture) {
  if (gesture & MLGestureStaticHandState_NoHand) {
    return 0;
  } else if (gesture & MLGestureStaticHandState_Finger) {
    return 1;
  } else if (gesture & MLGestureStaticHandState_Fist) {
    return 2;
  } else if (gesture & MLGestureStaticHandState_Pinch) {
    return 3;
  } else if (gesture & MLGestureStaticHandState_Thumb) {
    return 4;
  } else if (gesture & MLGestureStaticHandState_L) {
    return 5;
  } else if (gesture & MLGestureStaticHandState_OpenHandBack) {
    return 6;
  } else if (gesture & MLGestureStaticHandState_Ok) {
    return 7;
  } else if (gesture & MLGestureStaticHandState_C) {
    return 8;
  } else {
    return -1;
  }
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

MLContext::MLContext() :
  planesFloorQueryHandle(ML_INVALID_HANDLE),
  planesWallQueryHandle(ML_INVALID_HANDLE),
  planesCeilingQueryHandle(ML_INVALID_HANDLE),
  numFloorPlanes(0),
  numWallPlanes(0),
  numCeilingPlanes(0)
  {}

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

  glGenFramebuffers(1, &mlContext->framebuffer_id);

  makePlanesQueryer(mlContext->planesFloorHandle);
  makePlanesQueryer(mlContext->planesWallHandle);
  makePlanesQueryer(mlContext->planesCeilingHandle);

  MLInputConfiguration inputConfiguration;
  for (int i = 0; i < MLInput_MaxControllers; i++) {
    inputConfiguration.dof[i] = MLInputControllerDof_6;
  }
  mlContext->inputTracker = MLInputCreate(&inputConfiguration);
  if (!MLHandleIsValid(mlContext->inputTracker)) {
    ML_LOG(Error, "%s: Failed to create input tracker.", application_name);
  }

  mlContext->gestureTracker = MLGestureTrackingCreate();
  if (!MLHandleIsValid(mlContext->gestureTracker)) {
    ML_LOG(Error, "%s: Failed to create gesture tracker.", application_name);
  }

  MLMeshingSettings meshingSettings;
  meshingSettings.bounds_center.x = 0;
  meshingSettings.bounds_center.y = 0;
  meshingSettings.bounds_center.z = 0;
  meshingSettings.bounds_rotation.x = 0;
  meshingSettings.bounds_rotation.y = 0;
  meshingSettings.bounds_rotation.z = 0;
  meshingSettings.bounds_rotation.w = 1;
  meshingSettings.bounds_extents.x = 10;
  meshingSettings.bounds_extents.y = 10;
  meshingSettings.bounds_extents.z = 10;
  meshingSettings.compute_normals = true;
  meshingSettings.disconnected_component_area = 0.1;
  meshingSettings.enable_meshing = true;
  meshingSettings.fill_hole_length = 0.1;
  meshingSettings.fill_holes = false;
  meshingSettings.index_order_ccw = false;
  meshingSettings.mesh_type = MLMeshingType_Full;
  // meshingSettings.mesh_type = MLMeshingType_PointCloud;
  // meshingSettings.mesh_type = MLMeshingType_Blocks;
  meshingSettings.meshing_poll_time = 1e9;
  meshingSettings.planarize = false;
  meshingSettings.remove_disconnected_components = false;
  meshingSettings.remove_mesh_skirt = false;
  meshingSettings.request_vertex_confidence = false;
  meshingSettings.target_number_triangles = 0;
  meshingSettings.target_number_triangles_per_block = 0;
  mlContext->meshTracker = MLMeshingCreate(&meshingSettings);

  MLDataArrayInitDiff(&mlContext->meshesDataDiff);
  MLDataArrayInitDiff(&mlContext->meshesDataDiff2);

  info.GetReturnValue().Set(JS_BOOL(true));
}

NAN_METHOD(MLContext::WaitGetPoses) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  if (info[0]->IsUint32Array() && info[1]->IsFloat32Array() && info[2]->IsFloat32Array() && info[3]->IsUint32Array() && info[4]->IsFloat32Array() && info[5]->IsUint32Array() && info[6]->IsFloat32Array() && info[7]->IsFloat32Array() && info[8]->IsArray()) {
    if (application_context.dummy_value) {
      Local<Uint32Array> framebufferArray = Local<Uint32Array>::Cast(info[0]);
      Local<Float32Array> transformArray = Local<Float32Array>::Cast(info[1]);
      Local<Float32Array> projectionArray = Local<Float32Array>::Cast(info[2]);
      Local<Uint32Array> viewportArray = Local<Uint32Array>::Cast(info[3]);
      Local<Float32Array> planesArray = Local<Float32Array>::Cast(info[4]);
      Local<Uint32Array> numPlanesArray = Local<Uint32Array>::Cast(info[5]);
      Local<Float32Array> controllersArray = Local<Float32Array>::Cast(info[6]);
      Local<Float32Array> gesturesArray = Local<Float32Array>::Cast(info[7]);
      Local<Array> meshArray = Local<Array>::Cast(info[8]);

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

      // framebuffer
      framebufferArray->Set(0, JS_INT((unsigned int)mlContext->virtual_camera_array.color_id));
      framebufferArray->Set(1, JS_INT((unsigned int)mlContext->virtual_camera_array.depth_id));

      // transform
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

      // viewport
      const MLRectf& viewport = mlContext->virtual_camera_array.viewport;
      viewportArray->Set(0, JS_INT((int)viewport.x));
      viewportArray->Set(1, JS_INT((int)viewport.y));
      viewportArray->Set(2, JS_INT((unsigned int)viewport.w));
      viewportArray->Set(3, JS_INT((unsigned int)viewport.h));

      // planes
      endPlanesQuery(mlContext->planesFloorHandle, mlContext->planesFloorQueryHandle, mlContext->floorPlanes, &mlContext->numFloorPlanes);
      endPlanesQuery(mlContext->planesWallHandle, mlContext->planesWallQueryHandle, mlContext->wallPlanes, &mlContext->numWallPlanes);
      endPlanesQuery(mlContext->planesCeilingHandle, mlContext->planesCeilingQueryHandle, mlContext->ceilingPlanes, &mlContext->numCeilingPlanes);

      uint32_t planesIndex = 0;
      readPlanesQuery(mlContext->floorPlanes, mlContext->numFloorPlanes, 0, planesArray, &planesIndex);
      readPlanesQuery(mlContext->wallPlanes, mlContext->numWallPlanes, 1, planesArray, &planesIndex);
      readPlanesQuery(mlContext->ceilingPlanes, mlContext->numCeilingPlanes, 2, planesArray, &planesIndex);
      numPlanesArray->Set(0, JS_INT((int)planesIndex));

      // controllers
      MLInputControllerState controllerStates[MLInput_MaxControllers];
      if (MLInputGetControllerState(mlContext->inputTracker, controllerStates)) {
        for (int i = 0; i < 2 && i < MLInput_MaxControllers; i++) {
          MLInputControllerState &controllerState = controllerStates[i];
          MLVec3f &position = controllerState.position;
          MLQuaternionf &orientation = controllerState.orientation;

          controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 0, JS_NUM(position.x));
          controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 1, JS_NUM(position.y));
          controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 2, JS_NUM(position.z));
          controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 3, JS_NUM(orientation.x));
          controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 4, JS_NUM(orientation.y));
          controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 5, JS_NUM(orientation.z));
          controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 6, JS_NUM(orientation.w));
        }
      } else {
        ML_LOG(Error, "MLInputGetControllerState failed: %s", application_name);
      }

      // gestures
      MLGestureData gestureData;
      if (MLGestureGetData(mlContext->gestureTracker, &gestureData)) {
        MLGestureOneHandedState &leftHand = gestureData.left_hand_state;
        MLVec3f &leftCenter = leftHand.hand_center_normalized;
        gesturesArray->Set(0*4 + 0, JS_NUM(leftCenter.x));
        gesturesArray->Set(0*4 + 1, JS_NUM(leftCenter.y));
        gesturesArray->Set(0*4 + 2, JS_NUM(leftCenter.z));
        gesturesArray->Set(0*4 + 3, JS_NUM(gestureCategoryToIndex(leftHand.static_gesture_category)));

        MLGestureOneHandedState &rightHand = gestureData.right_hand_state;
        MLVec3f &rightCenter = rightHand.hand_center_normalized;
        gesturesArray->Set(1*4 + 0, JS_NUM(rightCenter.x));
        gesturesArray->Set(1*4 + 1, JS_NUM(rightCenter.y));
        gesturesArray->Set(1*4 + 2, JS_NUM(rightCenter.z));
        gesturesArray->Set(1*4 + 3, JS_NUM(gestureCategoryToIndex(rightHand.static_gesture_category)));
      } else {
        ML_LOG(Error, "MLGestureGetData failed: %s", application_name);
      }

      // meshing
      MLMeshingStaticData meshStaticData;
      if (MLMeshingGetStaticData(mlContext->meshTracker, &meshStaticData)) {
        MLCoordinateFrameUID coordinateFrame = meshStaticData.frame;
        MLDataArrayHandle &meshesHandle = meshStaticData.meshes;

        MLDataArrayLockResult lockResult = MLDataArrayTryLock(meshesHandle, &mlContext->meshData, &mlContext->meshesDataDiff);
        if (lockResult == MLDataArrayLockResult_New) {
          if (mlContext->meshData.stream_count > 0) {
            MLDataArrayStream &handleStream = mlContext->meshData.streams[0];

            if (handleStream.type == MLDataArrayType_Handle) {
              MLDataArrayHandle &meshesHandle2 = *handleStream.handle_array;

              MLDataArrayLockResult lockResult2 = MLDataArrayTryLock(meshesHandle2, &mlContext->meshData2, &mlContext->meshesDataDiff2);
              if (lockResult2 == MLDataArrayLockResult_New) {
                uint32_t normalIndex = meshStaticData.normal_stream_index;
                uint32_t positionIndex = meshStaticData.position_stream_index;
                uint32_t triangleIndex = meshStaticData.triangle_index_stream_index;

                MLDataArrayStream &normalStream = mlContext->meshData2.streams[normalIndex];
                uint32_t numNormals = normalStream.count;
                uint32_t normalsSize = numNormals * normalStream.data_size;
                mlContext->normals.resize(normalsSize);
                memcpy(mlContext->normals.data(), normalStream.custom_array, normalsSize);

                MLDataArrayStream &positionStream = mlContext->meshData2.streams[positionIndex];
                uint32_t numPositions = positionStream.count;
                uint32_t positionsSize = numPositions * positionStream.data_size;
                mlContext->positions.resize(positionsSize);
                memcpy(mlContext->positions.data(), positionStream.custom_array, positionsSize);

                MLDataArrayStream &triangleStream = mlContext->meshData2.streams[triangleIndex];
                uint32_t numTriangles = triangleStream.count;
                uint32_t trianglesSize = numTriangles * triangleStream.data_size;
                mlContext->triangles.resize(trianglesSize);
                memcpy(mlContext->triangles.data(), triangleStream.custom_array, trianglesSize);

                if (!meshArray->Get(0)->IsFloat32Array() || Local<Float32Array>::Cast(meshArray->Get(0))->Length() != numPositions) {
                  Local<ArrayBuffer> positionsArrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), mlContext->positions.data(), numPositions);
                  Local<Float32Array> positionsFloat32Array = Float32Array::New(positionsArrayBuffer, 0, numPositions);
                  meshArray->Set(0, positionsFloat32Array);
                }
                if (!meshArray->Get(1)->IsFloat32Array() || Local<Float32Array>::Cast(meshArray->Get(1))->Length() != numNormals) {
                  Local<ArrayBuffer> normalsArrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), mlContext->normals.data(), numNormals);
                  Local<Float32Array> normalsFloat32Array = Float32Array::New(normalsArrayBuffer, 0, numNormals);
                  meshArray->Set(1, normalsFloat32Array);
                }
                if (!meshArray->Get(2)->IsUint32Array() || Local<Uint32Array>::Cast(meshArray->Get(2))->Length() != numTriangles) {
                  Local<ArrayBuffer> trianglesArrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), mlContext->triangles.data(), numTriangles);
                  Local<Uint32Array> trianglesUint32Array = Uint32Array::New(trianglesArrayBuffer, 0, numTriangles);
                  meshArray->Set(2, trianglesUint32Array);
                }

                MLDataArrayUnlock(meshesHandle2);
              } else if (lockResult2 == MLDataArrayLockResult_Unchanged) {
                // nothing
              } else {
                ML_LOG(Error, "MLDataArrayTryLock inner failed: %d", lockResult);
              }
            } else {
              ML_LOG(Error, "invalid handle stream type: %d", handleStream.type);
            }
          } else {
            ML_LOG(Error, "invalid stream count: %d", mlContext->meshData.stream_count);
          }

          MLDataArrayUnlock(meshesHandle);
        } else if (lockResult == MLDataArrayLockResult_Unchanged) {
          // nothing
        } else {
          ML_LOG(Error, "MLDataArrayTryLock outer failed: %d", lockResult);
        }
      } else {
        ML_LOG(Error, "MLMeshingGetStaticData failed: %s", application_name);
      }
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

    glBindFramebuffer(GL_READ_FRAMEBUFFER, src_framebuffer_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->framebuffer_id);

    glBlitFramebuffer(camera == 0 ? 0 : width/2, 0,
      camera == 0 ? width/2 : width, height,
      viewport.x, viewport.y,
      viewport.w, viewport.h,
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

  beginPlanesQuery(mlContext->planesFloorHandle, mlContext->planesFloorQueryHandle, static_cast<MLPlanesQueryFlags>(MLPlanesQueryFlag_AllOrientations | MLPlanesQueryFlag_Semantic_Floor));
  beginPlanesQuery(mlContext->planesWallHandle, mlContext->planesWallQueryHandle, static_cast<MLPlanesQueryFlags>(MLPlanesQueryFlag_AllOrientations | MLPlanesQueryFlag_Semantic_Wall));
  beginPlanesQuery(mlContext->planesCeilingHandle, mlContext->planesCeilingQueryHandle, static_cast<MLPlanesQueryFlags>(MLPlanesQueryFlag_AllOrientations | MLPlanesQueryFlag_Semantic_Ceiling));
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
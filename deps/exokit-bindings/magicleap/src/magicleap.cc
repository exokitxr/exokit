#if defined(MAGICLEAP) || defined(LUMIN)

#include <magicleap.h>
#include <uv.h>
#include <iostream>

using namespace v8;
using namespace std;

namespace ml {

const char application_name[] = "com.exokit.app";
application_context_t application_context;
MLResult lifecycle_status = MLResult_Pending;
Nan::Persistent<Function> initCb;

MLLifecycleCallbacks lifecycle_callbacks;
MLInputKeyboardCallbacks keyboardCallbacks;
MLCameraDeviceStatusCallbacks cameraDeviceStatusCallbacks;
MLCameraCaptureCallbacks cameraCaptureCallbacks;

Nan::Persistent<Function> eventsCb;
std::vector<Event> events;
uv_async_t eventsAsync;

Nan::Persistent<Function> keyboardEventsCb;
std::vector<KeyboardEvent> keyboardEvents;
uv_async_t keyboardEventsAsync;

std::thread cameraRequestThread;
std::mutex cameraRequestMutex;
std::condition_variable  cameraRequestConditionVariable;
std::mutex cameraRequestsMutex;
std::vector<CameraRequest *> cameraRequests;
bool cameraResponsePending = false;

MLHandle handTracker;
MLHandTrackingData handData;
std::vector<HandRequest *> handRequests;

MLHandle meshTracker;
std::vector<MeshRequest *> meshRequests;
MLMeshingExtents meshExtents;
MLHandle meshInfoRequestHandle;
MLMeshingMeshInfo meshInfo;
bool meshInfoRequestPending = false;
std::vector<MLMeshingBlockRequest> meshBlockRequests(128);
uint32_t numMeshBlockRequests = 0;
uint32_t meshBlockRequestIndex = 0;
MLMeshingMeshRequest meshRequest;
std::vector<MLCoordinateFrameUID> meshRemovedList;
MLHandle meshRequestHandle;
MLMeshingMesh mesh;
bool meshRequestsPending = false;
bool meshRequestPending = false;

MLHandle planesTracker;
std::vector<PlanesRequest *> planesRequests;
MLPlanesQuery planesRequest;
MLHandle planesRequestHandle;
MLPlane planeResults[MAX_NUM_PLANES];
uint32_t numPlanesResults;
bool planesRequestPending = false;

MLHandle eyeTracker;
std::vector<EyeRequest *> eyeRequests;
MLEyeTrackingState eyeState;
MLEyeTrackingStaticData eyeStaticData;

bool isPresent() {
  return lifecycle_status == MLResult_Ok;
}

bool isSimulated() {
#ifdef LUMIN
  return false;
#else
  return true;
#endif
}

/* void makePlanesQueryer(MLHandle &planesHandle) {
  if (MLPlanesCreate(&planesHandle) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to create planes handle.", application_name);
  }
}
void beginPlanesQuery(const MLVec3f &position, const MLQuaternionf &rotation, MLHandle &planesHandle, MLHandle &planesQueryHandle, MLPlanesQueryFlags flags) {
  if (!MLHandleIsValid(planesQueryHandle)) {
    MLPlanesQuery query;
    query.flags = flags;
    // query.bounds_center = position;
    // query.bounds_rotation = rotation;
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

    if (MLPlanesQueryBegin(planesHandle, &query, &planesQueryHandle) != MLResult_Ok) {
      ML_LOG(Error, "%s: Failed to query planes.", application_name);
    }
  }
}
void endPlanesQuery(MLHandle &planesHandle, MLHandle &planesQueryHandle, MLPlane *planes, uint32_t *numPlanes) {
  if (MLHandleIsValid(planesQueryHandle)) {
    MLResult result = MLPlanesQueryGetResults(planesHandle, planesQueryHandle, planes, numPlanes);
    if (result == MLResult_Ok) {
      planesQueryHandle = ML_INVALID_HANDLE;
    } else if (result == MLResult_UnspecifiedFailure) {
      planesQueryHandle = ML_INVALID_HANDLE;

      ML_LOG(Error, "MLPlanesQueryGetResults failed: %d", result);
    } else if (result == MLResult_Pending) {
      // nothing, we wait
    } else {
      ML_LOG(Error, "MLPlanesQueryGetResults complained: %d", result);
    }
  }
}
void readPlanesQuery(MLPlane *planes, uint32_t numPlanes, int planeType, Local<Float32Array> &planesArray, uint32_t *planesIndex) {
  for (uint32_t i = 0; i < numPlanes; i++) {
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
} */
inline int gestureCategoryToIndex(MLGestureStaticHandState gesture) {
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
inline const char *gestureCategoryToDescriptor(MLHandTrackingKeyPose keyPose) {
  switch (keyPose) {
    case MLGestureStaticHandState_NoHand:
      return nullptr;
    case MLGestureStaticHandState_Finger:
      return "finger";
    case MLGestureStaticHandState_Fist:
      return "fist";
    case MLGestureStaticHandState_Pinch:
      return "punch";
    case MLGestureStaticHandState_Thumb:
      return "thumb";
    case MLGestureStaticHandState_L:
      return "l";
    case MLGestureStaticHandState_OpenHandBack:
      return "openHandBack";
    case MLGestureStaticHandState_Ok:
      return "ok";
    case MLGestureStaticHandState_C:
      return "c";
    default:
      return nullptr;
  }
}

static void onNewInitArg(void* application_context) {
  MLLifecycleInitArgList *args;
  MLLifecycleGetInitArgList(&args);

  // ((struct application_context_t*)application_context)->dummy_value = DummyValue::RUNNING;
  ML_LOG(Info, "%s: On new init arg called %x.", application_name, args);

  events.push_back(Event::NEW_INIT_ARG);
  uv_async_send(&eventsAsync);
}

static void onStop(void* application_context) {
  // ((struct application_context_t*)application_context)->dummy_value = DummyValue::STOPPED;
  ML_LOG(Info, "%s: On stop called.", application_name);
  events.push_back(Event::STOP);
  uv_async_send(&eventsAsync);
}

static void onPause(void* application_context) {
  // ((struct application_context_t*)application_context)->dummy_value = DummyValue::PAUSED;
  ML_LOG(Info, "%s: On pause called.", application_name);
  events.push_back(Event::PAUSE);
  uv_async_send(&eventsAsync);
}

static void onResume(void* application_context) {
  // ((struct application_context_t*)application_context)->dummy_value = DummyValue::RUNNING;
  ML_LOG(Info, "%s: On resume called.", application_name);
  events.push_back(Event::RESUME);
  uv_async_send(&eventsAsync);
}

static void onUnloadResources(void* application_context) {
  // ((struct application_context_t*)application_context)->dummy_value = DummyValue::STOPPED;
  ML_LOG(Info, "%s: On unload resources called.", application_name);
  events.push_back(Event::UNLOAD_RESOURCES);
  uv_async_send(&eventsAsync);
}

// HandRequest

HandRequest::HandRequest(Local<Function> cbFn) : cbFn(cbFn) {}

constexpr float HAND_CONFIDENCE = 0.5;
void HandRequest::Poll() {
  Local<Object> asyncObject = Nan::New<Object>();
  AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "handRequest");

  Local<Array> array = Nan::New<Array>();
  uint32_t numResults = 0;

  const MLHandTrackingHandState &leftHandState = handData.left_hand_state;
  if (leftHandState.hand_confidence >= HAND_CONFIDENCE) {
    const MLVec3f &center = leftHandState.hand_center_normalized;
    const MLHandTrackingKeyPose &keypose = leftHandState.keypose;

    Local<Object> obj = Nan::New<Object>();

    obj->Set(JS_STR("hand"), JS_STR("left"));

    Local<Float32Array> centerArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)center.values, 3 * sizeof(float)), 0, 3);
    obj->Set(JS_STR("center"), centerArray);

    const char *gesture = gestureCategoryToDescriptor(keypose);
    if (gesture) {
      obj->Set(JS_STR("gesture"), JS_STR(gesture));
    } else {
      obj->Set(JS_STR("gesture"), Nan::Null());
    }

    array->Set(JS_INT(numResults++), obj);
  }

  const MLHandTrackingHandState &rightHandState = handData.right_hand_state;
  if (rightHandState.hand_confidence >= HAND_CONFIDENCE) {
    const MLVec3f &center = rightHandState.hand_center_normalized;
    const MLHandTrackingKeyPose &keypose = rightHandState.keypose;

    Local<Object> obj = Nan::New<Object>();

    obj->Set(JS_STR("hand"), JS_STR("right"));

    Local<Float32Array> centerArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)center.values, 3 * sizeof(float)), 0, 3);
    obj->Set(JS_STR("center"), centerArray);

    const char *gesture = gestureCategoryToDescriptor(keypose);
    if (gesture) {
      obj->Set(JS_STR("gesture"), JS_STR(gesture));
    } else {
      obj->Set(JS_STR("gesture"), Nan::Null());
    }

    array->Set(JS_INT(numResults++), obj);
  }

  Local<Function> cbFn = Nan::New(this->cbFn);
  Local<Value> argv[] = {
    array,
  };
  asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);
}

// MeshBuffer

MeshBuffer::MeshBuffer(GLuint positionBuffer, GLuint normalBuffer, GLuint indexBuffer) :
  positionBuffer(positionBuffer),
  normalBuffer(normalBuffer),
  indexBuffer(indexBuffer)
  {}
MeshBuffer::MeshBuffer(const MeshBuffer &meshBuffer) {
  positionBuffer = meshBuffer.positionBuffer;
  normalBuffer = meshBuffer.normalBuffer;
  indexBuffer = meshBuffer.indexBuffer;
}
MeshBuffer::MeshBuffer() : positionBuffer(0), normalBuffer(0), indexBuffer(0) {}

void MeshBuffer::setBuffers(float *positions, uint32_t numPositions, float *normals, unsigned short *indices, uint16_t numIndices) {
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, numPositions, positions, GL_DYNAMIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
  glBufferData(GL_ARRAY_BUFFER, numPositions, normals, GL_DYNAMIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
  glBufferData(GL_ARRAY_BUFFER, numIndices, indices, GL_DYNAMIC_DRAW);
}

// MeshRequest

MeshRequest::MeshRequest(Local<Function> cbFn) : cbFn(cbFn) {}

void MeshRequest::Poll() {
  Local<Object> asyncObject = Nan::New<Object>();
  AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "meshRequest");

  MLMeshingBlockMesh *blockMeshes = mesh.data;
  uint32_t dataCount = mesh.data_count;

  Local<Array> array = Nan::New<Array>();
  uint32_t numResults = 0;
  for (uint32_t i = 0; i < dataCount; i++) {
    MLMeshingBlockMesh &blockMesh = blockMeshes[i];

    const MLMeshingResult &result = blockMesh.result;
    if (result == MLMeshingResult_Success || result == MLMeshingResult_PartialUpdate) {
      uint64_t id1 = blockMesh.id.data[0];
      uint64_t id2 = blockMesh.id.data[1];
      char idbuf[16*2 + 1];
      sprintf(idbuf, "%016llx%016llx", id1, id2);
      std::string id(idbuf);

      MeshBuffer *meshBuffer;
      auto iter = meshBuffers.find(id);
      if (iter != meshBuffers.end()) {
        meshBuffer = &iter->second;
      } else {
        GLuint buffers[3];
        glGenBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
        meshBuffers[id] = MeshBuffer(buffers[0], buffers[1], buffers[2]);
        meshBuffer = &meshBuffers[id];
      }
      meshBuffer->setBuffers((float *)blockMesh.vertex, blockMesh.vertex_count, (float *)blockMesh.normal, blockMesh.index, blockMesh.index_count);
      
      Local<Object> obj = Nan::New<Object>();
      obj->Set(JS_STR("id"), JS_STR(id));
      Local<Object> positionObj = Nan::New<Object>();
      positionObj->Set(JS_STR("id"), JS_INT(meshBuffer->positionBuffer));
      obj->Set(JS_STR("position"), positionObj);
      Local<Object> normalObj = Nan::New<Object>();
      normalObj->Set(JS_STR("id"), JS_INT(meshBuffer->normalBuffer));
      obj->Set(JS_STR("normal"), normalObj);
      Local<Object> indexObj = Nan::New<Object>();
      indexObj->Set(JS_STR("id"), JS_INT(meshBuffer->indexBuffer));
      obj->Set(JS_STR("index"), indexObj);
      obj->Set(JS_STR("valid"), JS_BOOL(true));

      array->Set(numResults++, obj);
    } else if (result == MLMeshingResult_Pending) {
      // nothing
    } else {
      ML_LOG(Error, "%s: ML mesh request poll failed: %x %x", application_name, i, result);
    }
  }
  for (const MLCoordinateFrameUID &cfid : meshRemovedList) {
    uint64_t id1 = cfid.data[0];
    uint64_t id2 = cfid.data[1];
    char idbuf[16*2 + 1];
    sprintf(idbuf, "%016llx%016llx", id1, id2);
    std::string id(idbuf);

    auto iter = meshBuffers.find(id);
    if (iter != meshBuffers.end()) {
      MeshBuffer &meshBuffer = iter->second;
      GLuint buffers[3] = {
        meshBuffer.positionBuffer,
        meshBuffer.normalBuffer,
        meshBuffer.indexBuffer,
      };
      glDeleteBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
      meshBuffers.erase(iter);
    }

    Local<Object> obj = Nan::New<Object>();
    obj->Set(JS_STR("id"), JS_STR(id));
    obj->Set(JS_STR("valid"), JS_BOOL(false));

    array->Set(numResults++, obj);
  }

  Local<Function> cbFn = Nan::New(this->cbFn);
  Local<Value> argv[] = {
    array,
  };
  asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);
}

// PlanesRequest

PlanesRequest::PlanesRequest(Local<Function> cbFn) : cbFn(cbFn) {}

void PlanesRequest::Poll() {
  Local<Object> asyncObject = Nan::New<Object>();
  AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "planesRequest");

  Local<Array> array = Nan::New<Array>(numPlanesResults);
  for (uint32_t i = 0; i < numPlanesResults; i++) {
    MLPlane &plane = planeResults[i];

    Local<Object> obj = Nan::New<Object>();

    uint64_t id = (uint64_t)plane.id;
    // uint32_t flags = plane.flags;
    float width = plane.width;
    float height = plane.height;
    MLVec3f &position = plane.position;
    MLQuaternionf &rotation = plane.rotation;

    char s[16 + 1];
    sprintf(s, "%016llx", id);
    obj->Set(JS_STR("id"), JS_STR(s));

    obj->Set(JS_STR("width"), JS_NUM(width));
    obj->Set(JS_STR("height"), JS_NUM(height));

    Local<Float32Array> positionArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 3 * sizeof(float)), 0, 3);
    positionArray->Set(0, JS_NUM(position.x));
    positionArray->Set(1, JS_NUM(position.y));
    positionArray->Set(2, JS_NUM(position.z));
    obj->Set(JS_STR("position"), positionArray);

    Local<Float32Array> rotationArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 4 * sizeof(float)), 0, 4);
    rotationArray->Set(0, JS_NUM(rotation.x));
    rotationArray->Set(1, JS_NUM(rotation.y));
    rotationArray->Set(2, JS_NUM(rotation.z));
    rotationArray->Set(3, JS_NUM(rotation.w));
    obj->Set(JS_STR("rotation"), rotationArray);

    array->Set(i, obj);
  }

  Local<Function> cbFn = Nan::New(this->cbFn);
  Local<Value> argv[] = {
    array,
  };
  asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);
}

// EyeRequest

EyeRequest::EyeRequest(Local<Function> cbFn) : cbFn(cbFn) {}

void EyeRequest::Poll() {
  MLSnapshot *snapshot;
  if (MLPerceptionGetSnapshot(&snapshot) != MLResult_Ok) {
    ML_LOG(Error, "%s: ML failed to get eye snapshot!", application_name);
  }

  const MLCoordinateFrameUID &id = eyeStaticData.fixation;
  MLTransform transform;
  if (MLSnapshotGetTransform(snapshot, &id, &transform) == MLResult_Ok) {
    Local<Object> asyncObject = Nan::New<Object>();
    AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "eyeRequest");

    Local<Float32Array> fixationTransformArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (3 + 4) * sizeof(float)), 0, 3 + 4);
    fixationTransformArray->Set(0, JS_NUM(transform.position.x));
    fixationTransformArray->Set(1, JS_NUM(transform.position.y));
    fixationTransformArray->Set(2, JS_NUM(transform.position.z));
    fixationTransformArray->Set(3, JS_NUM(transform.rotation.x));
    fixationTransformArray->Set(4, JS_NUM(transform.rotation.y));
    fixationTransformArray->Set(5, JS_NUM(transform.rotation.z));
    fixationTransformArray->Set(6, JS_NUM(transform.rotation.w));

    Local<Function> cbFn = Nan::New(this->cbFn);
    Local<Value> argv[] = {
      fixationTransformArray,
    };
    asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);
  } else {
    ML_LOG(Error, "%s: ML failed to get eye transform!", application_name);
  }

  if (MLPerceptionReleaseSnapshot(snapshot) != MLResult_Ok) {
    ML_LOG(Error, "%s: ML failed to release eye snapshot!", application_name);
  }
}

// keyboard callbacks

void onChar(uint32_t char_utf32, void *data) {
  keyboardEvents.push_back(KeyboardEvent(KeyboardEventType::CHAR, char_utf32));
  uv_async_send(&keyboardEventsAsync);
}
void onKeyDown(MLKeyCode key_code, uint32_t modifier_mask, void *data) {
  keyboardEvents.push_back(KeyboardEvent(KeyboardEventType::KEY_DOWN, key_code, modifier_mask));
  uv_async_send(&keyboardEventsAsync);
}
void onKeyUp(MLKeyCode key_code, uint32_t modifier_mask, void *data) {
  keyboardEvents.push_back(KeyboardEvent(KeyboardEventType::KEY_UP, key_code, modifier_mask));
  uv_async_send(&keyboardEventsAsync);
}

// KeyboardEvent

KeyboardEvent::KeyboardEvent(KeyboardEventType type, uint32_t char_utf32) : type(type), char_utf32(char_utf32), key_code(MLKEYCODE_UNKNOWN), modifier_mask(0) {}
KeyboardEvent::KeyboardEvent(KeyboardEventType type, MLKeyCode key_code, uint32_t modifier_mask) : type(type), char_utf32(0), key_code(key_code), modifier_mask(modifier_mask) {}

// camera device status callbacks

void cameraOnDeviceAvailable(void *data) {
  // XXX
}
void cameraOnDeviceUnavailable(void *data) {
  // XXX
}
void cameraOnDeviceOpened(void *data) {
  // XXX
}
void cameraOnDeviceClosed(void *data) {
  // XXX
}
void cameraOnDeviceDisconnected(void *data) {
  // XXX
}
void cameraOnDeviceError(MLCameraError error, void *data) {
  // XXX
}
void cameraOnPreviewBufferAvailable(MLHandle output, void *data) {
  /* std::cout << "camera preview buffer available " << cameraRequests.size() << " " << cameraResponsePending << " " << output << std::endl;

  if (!cameraResponsePending) {
    std::cout << "camera preview buffer num planes " << output << std::endl;

    std::for_each(cameraRequests.begin(), cameraRequests.end(), [&](CameraRequest *c) {
      c->Set(output);
    });
    cameraResponsePending = true;
  } */
}

// camera capture callbacks

void cameraOnCaptureStarted(const MLCameraResultExtras *extra, void *data) {
  // XXX
}
void cameraOnCaptureFailed(const MLCameraResultExtras *extra, void *data) {
  // XXX
}
void cameraOnCaptureBufferLost(const MLCameraResultExtras *extra, void *data) {
  // XXX
}
void cameraOnCaptureProgressed(MLHandle metadata_handle, const MLCameraResultExtras *extra, void *data) {
  // XXX
}
void cameraOnCaptureCompleted(MLHandle metadata_handle, const MLCameraResultExtras *extra, void *data) {
  // XXX
}
void cameraOnImageBufferAvailable(const MLCameraOutput *output, void *data) {
  if (!cameraResponsePending) {
    {
      std::unique_lock<std::mutex> lock(cameraRequestsMutex);
      std::for_each(cameraRequests.begin(), cameraRequests.end(), [&](CameraRequest *c) {
        c->Set(output);
      });
    }
    cameraResponsePending = true;
  }
}

// CameraRequestPlane

constexpr uint32_t previewSize = 456192;
CameraRequestPlane::CameraRequestPlane(uint32_t width, uint32_t height, uint8_t *dataArg, uint32_t size, uint32_t bpp, uint32_t stride) : width(width), height(height), size(size), bpp(bpp), stride(stride) {
  memcpy(data, dataArg, size);
}

CameraRequestPlane::CameraRequestPlane(MLHandle output) : width(0), height(0), size(previewSize), bpp(0), stride(0) {
  memcpy(data, (void *)output, previewSize);
}

void CameraRequestPlane::set(uint32_t width, uint32_t height, uint8_t *dataArg, uint32_t size, uint32_t bpp, uint32_t stride) {
  this->width = width;
  this->height = height;
  if (size > sizeof(data)) {
    ML_LOG(Error, "%s: ML camera request plane overflow! %x %x", application_name, size, sizeof(data));
  }
  memcpy(data, dataArg, size);
  this->bpp = bpp;
  this->stride = stride;
}

void CameraRequestPlane::set(MLHandle output) {
  uint32_t size = previewSize;
  if (size > sizeof(data)) {
    ML_LOG(Error, "%s: ML camera request plane overflow! %x %x", application_name, size, sizeof(data));
  }
  memcpy(data, (void *)output, size);
}

// CameraRequest

CameraRequest::CameraRequest(MLHandle request, Local<Function> cbFn) : request(request), cbFn(cbFn) {}

void CameraRequest::Set(const MLCameraOutput *output) {
  const MLCameraOutputFormat &format = output->format;
  uint8_t planeCount = output->plane_count;
  const MLCameraPlaneInfo *planes = output->planes;

  for (uint8_t i = 0; i < planeCount; i++) {
    const MLCameraPlaneInfo &plane = planes[i];

    uint32_t width = plane.width;
    uint32_t height = plane.height;
    uint8_t *data = plane.data;
    uint32_t size = plane.size;
    uint32_t bpp = plane.bytes_per_pixel;
    uint32_t stride = plane.stride;

    // std::cout << "got plane " << width << " " << height << " " << size << " " << bpp << " " << stride << std::endl;

    if (i < this->planes.size()) {
      this->planes[i]->set(width, height, data, size, bpp, stride);
    } else {
      this->planes.push_back(new CameraRequestPlane(width, height, data, size, bpp, stride));
    }
  }
}

void CameraRequest::Set(MLHandle output) {
  if (0 < this->planes.size()) {
    this->planes[0]->set(output);
  } else {
    this->planes.push_back(new CameraRequestPlane(output));
  }
}

void CameraRequest::Poll(WebGLRenderingContext *gl, GLuint fbo, unsigned int width, unsigned int height) {
  size_t planeCount = planes.size();
  Local<Array> array = Nan::New<Array>(planeCount + 1);
  for (uint8_t i = 0; i < planeCount; i++) {
    CameraRequestPlane *plane = planes[i];

    uint32_t width = plane->width;
    uint32_t height = plane->height;
    uint8_t *data = plane->data;
    uint32_t size = plane->size;
    uint32_t bpp = plane->bpp;
    uint32_t stride = plane->stride;

    Local<Object> obj = Nan::New<Object>();
    obj->Set(JS_STR("data"), ArrayBuffer::New(Isolate::GetCurrent(), data, size));
    obj->Set(JS_STR("width"), JS_INT(width));
    obj->Set(JS_STR("height"), JS_INT(height));
    obj->Set(JS_STR("bpp"), JS_INT(bpp));
    obj->Set(JS_STR("stride"), JS_INT(stride));
    array->Set(i, obj);
  }
  {
    uint8_t i = planeCount;
    Local<Object> obj = Nan::New<Object>();
    unsigned int halfWidth = width / 2;
    Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), halfWidth * height * 4);
    unsigned char *data = (unsigned char *)arrayBuffer->GetContents().Data();
    egl::ReadPixels(gl, fbo, 0, 0, halfWidth, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    obj->Set(JS_STR("data"), arrayBuffer);
    obj->Set(JS_STR("width"), JS_INT(halfWidth));
    obj->Set(JS_STR("height"), JS_INT(height));
    obj->Set(JS_STR("bpp"), JS_INT(4));
    obj->Set(JS_STR("stride"), JS_INT(halfWidth));
    array->Set(i, obj);
  }

  Local<Object> asyncObject = Nan::New<Object>();
  AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "cameraRequest");

  Local<Function> cbFn = Nan::New(this->cbFn);
  Local<Value> argv[] = {
    array,
  };
  asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);
}

MLContext::MLContext() :
  position{0, 0, 0},
  rotation{0, 0, 0, 1}
  {}

MLContext::~MLContext() {}

Handle<Object> MLContext::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "Present", Present);
  Nan::SetMethod(proto, "Exit", Exit);
  Nan::SetMethod(proto, "WaitGetPoses", WaitGetPoses);
  Nan::SetMethod(proto, "SubmitFrame", SubmitFrame);

  Local<Function> ctorFn = ctor->GetFunction();

  Nan::SetMethod(ctorFn, "InitLifecycle", InitLifecycle);
  Nan::SetMethod(ctorFn, "DeinitLifecycle", DeinitLifecycle);
  Nan::SetMethod(ctorFn, "IsPresent", IsPresent);
  Nan::SetMethod(ctorFn, "IsSimulated", IsSimulated);
  Nan::SetMethod(ctorFn, "OnPresentChange", OnPresentChange);
  Nan::SetMethod(ctorFn, "RequestHand", RequestHand);
  Nan::SetMethod(ctorFn, "CancelHand", CancelHand);
  Nan::SetMethod(ctorFn, "RequestMesh", RequestMesh);
  Nan::SetMethod(ctorFn, "CancelMesh", CancelMesh);
  Nan::SetMethod(ctorFn, "RequestPlanes", RequestPlanes);
  Nan::SetMethod(ctorFn, "CancelPlanes", CancelPlanes);
  Nan::SetMethod(ctorFn, "RequestEye", RequestEye);
  Nan::SetMethod(ctorFn, "CancelEye", CancelEye);
  Nan::SetMethod(ctorFn, "RequestCamera", RequestCamera);
  Nan::SetMethod(ctorFn, "CancelCamera", CancelCamera);
  Nan::SetMethod(ctorFn, "PrePollEvents", PrePollEvents);
  Nan::SetMethod(ctorFn, "PostPollEvents", PostPollEvents);

  return scope.Escape(ctorFn);
}

NAN_METHOD(MLContext::New) {
  Local<Object> mlContextObj = info.This();
  MLContext *mlContext = new MLContext();
  mlContext->Wrap(mlContextObj);

  info.GetReturnValue().Set(mlContextObj);
}

void RunEventsInMainThread(uv_async_t *handle) {
  Nan::HandleScope scope;

  for (const auto &event : events) {
    Local<Object> asyncObject = Nan::New<Object>();
    AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "mlEvents");

    Local<Value> arg;
    switch (event) {
      case Event::NEW_INIT_ARG: {
        arg = JS_STR("newInitArg");
        break;
      }
      case Event::STOP: {
        arg = JS_STR("stop");
        break;
      }
      case Event::PAUSE: {
        arg = JS_STR("pause");
        break;
      }
      case Event::RESUME: {
        arg = JS_STR("resume");
        break;
      }
      case Event::UNLOAD_RESOURCES: {
        arg = JS_STR("unloadResources");
        break;
      }
      default: {
        arg = Nan::Null();
        break;
      }
    }

    Local<Function> eventsCbFn = Nan::New(eventsCb);
    Local<Value> argv[] = {
      arg,
    };
    asyncResource.MakeCallback(eventsCbFn, sizeof(argv)/sizeof(argv[0]), argv);
  }
  events.clear();
}

inline uint32_t normalizeMLKeyCode(MLKeyCode mlKeyCode) {
  switch (mlKeyCode) {
    case MLKEYCODE_ENTER: return 13;
    case MLKEYCODE_ESCAPE: return 27;
    case MLKEYCODE_DEL: return 8;
    case MLKEYCODE_FORWARD_DEL: return 46;
    default: return (uint32_t)mlKeyCode;
  }
}

void RunKeyboardEventsInMainThread(uv_async_t *handle) {
  Nan::HandleScope scope;

  for (const auto &keyboardEvent : keyboardEvents) {
    Local<Object> asyncObject = Nan::New<Object>();
    AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "keyboardEvents");

    Local<Object> obj = Nan::New<Object>();

    Local<Value> typeArg;
    switch (keyboardEvent.type) {
      case KeyboardEventType::CHAR: {
        typeArg = JS_STR("keypresss");
        break;
      }
      case KeyboardEventType::KEY_DOWN: {
        typeArg = JS_STR("keydown");
        break;
      }
      case KeyboardEventType::KEY_UP: {
        typeArg = JS_STR("keyup");
        break;
      }
      default: {
        typeArg = Nan::Null();
        break;
      }
    }
    obj->Set(JS_STR("type"), typeArg);
    uint32_t charCode = (uint32_t)keyboardEvent.char_utf32;
    uint32_t keyCode = normalizeMLKeyCode(keyboardEvent.key_code);
    obj->Set(JS_STR("charCode"), JS_INT(charCode));
    obj->Set(JS_STR("keyCode"), JS_INT(keyCode));
    obj->Set(JS_STR("which"), JS_INT(keyCode));
    obj->Set(JS_STR("shiftKey"), JS_BOOL(keyboardEvent.modifier_mask & MLKEYMODIFIER_SHIFT));
    obj->Set(JS_STR("altKey"), JS_BOOL(keyboardEvent.modifier_mask & MLKEYMODIFIER_ALT));
    obj->Set(JS_STR("ctrlKey"), JS_BOOL(keyboardEvent.modifier_mask & MLKEYMODIFIER_CTRL));
    obj->Set(JS_STR("metaKey"), JS_BOOL(keyboardEvent.modifier_mask & MLKEYMODIFIER_META));
    // obj->Set(JS_STR("sym"), JS_BOOL(keyboardEvent.modifier_mask & MLKEYMODIFIER_SYM));
    // obj->Set(JS_STR("function"), JS_BOOL(keyboardEvent.modifier_mask & MLKEYMODIFIER_FUNCTION));
    // obj->Set(JS_STR("capsLock"), JS_BOOL(keyboardEvent.modifier_mask & MLKEYMODIFIER_CAPS_LOCK));
    // obj->Set(JS_STR("numLock"), JS_BOOL(keyboardEvent.modifier_mask & MLKEYMODIFIER_NUM_LOCK));
    // obj->Set(JS_STR("scrollLock"), JS_BOOL(keyboardEvent.modifier_mask & MLKEYMODIFIER_SCROLL_LOCK));

    Local<Function> keyboardEventsCbFn = Nan::New(keyboardEventsCb);
    Local<Value> argv[] = {
      obj,
    };
    asyncResource.MakeCallback(keyboardEventsCbFn, sizeof(argv)/sizeof(argv[0]), argv);
  }
  keyboardEvents.clear();
}

NAN_METHOD(MLContext::InitLifecycle) {
  std::cout << "init lifecycle" << std::endl;

  if (info[0]->IsFunction() && info[1]->IsFunction()) {
    eventsCb.Reset(Local<Function>::Cast(info[0]));
    keyboardEventsCb.Reset(Local<Function>::Cast(info[1]));

    uv_async_init(uv_default_loop(), &eventsAsync, RunEventsInMainThread);
    uv_async_init(uv_default_loop(), &keyboardEventsAsync, RunKeyboardEventsInMainThread);

    lifecycle_callbacks.on_new_initarg = onNewInitArg;
    lifecycle_callbacks.on_stop = onStop;
    lifecycle_callbacks.on_pause = onPause;
    lifecycle_callbacks.on_resume = onResume;
    lifecycle_callbacks.on_unload_resources = onUnloadResources;
    lifecycle_status = MLLifecycleInit(&lifecycle_callbacks, (void *)(&application_context));
    if (lifecycle_status != MLResult_Ok) {
      Nan::ThrowError("MLContext::InitLifecycle failed to initialize lifecycle");
      return;
    }

    {
      cameraDeviceStatusCallbacks.on_device_available = cameraOnDeviceAvailable;
      cameraDeviceStatusCallbacks.on_device_unavailable = cameraOnDeviceUnavailable;
      cameraDeviceStatusCallbacks.on_device_opened = cameraOnDeviceOpened;
      cameraDeviceStatusCallbacks.on_device_closed = cameraOnDeviceClosed;
      cameraDeviceStatusCallbacks.on_device_disconnected = cameraOnDeviceDisconnected;
      cameraDeviceStatusCallbacks.on_device_error = cameraOnDeviceError;
      cameraDeviceStatusCallbacks.on_preview_buffer_available = cameraOnPreviewBufferAvailable;
      MLResult result = MLCameraSetDeviceStatusCallbacks(&cameraDeviceStatusCallbacks, nullptr);
      if (result != MLResult_Ok) {
        ML_LOG(Error, "%s: failed to set camera device status callbacks %x", application_name, result);
        Nan::ThrowError("MLContext::InitLifecycle failed to set camera device status callbacks");
        return;
      }
    }

    {
      cameraCaptureCallbacks.on_capture_started = cameraOnCaptureStarted;
      cameraCaptureCallbacks.on_capture_failed = cameraOnCaptureFailed;
      cameraCaptureCallbacks.on_capture_buffer_lost = cameraOnCaptureBufferLost;
      cameraCaptureCallbacks.on_capture_progressed = cameraOnCaptureProgressed;
      cameraCaptureCallbacks.on_capture_completed = cameraOnCaptureCompleted;
      cameraCaptureCallbacks.on_image_buffer_available = cameraOnImageBufferAvailable;
      MLResult result = MLCameraSetCaptureCallbacks(&cameraCaptureCallbacks, nullptr);
      if (result != MLResult_Ok) {
        ML_LOG(Error, "%s: failed to set camera device status callbacks %x", application_name, result);
        Nan::ThrowError("MLContext::InitLifecycle failed to set camera device status callbacks");
        return;
      }
    }

    // HACK: prevent exit hang
    std::atexit([]() {
      quick_exit(0);
    });

    application_context.dummy_value = DummyValue::STOPPED;
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::DeinitLifecycle) {
  application_context.dummy_value = DummyValue::STOPPED;
}

NAN_METHOD(MLContext::Present) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));

  if (lifecycle_status != MLResult_Ok) {
    ML_LOG(Error, "%s: ML Present called before lifecycle initialized.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  MLGraphicsOptions graphics_options = {MLGraphicsFlags_Default, MLSurfaceFormat_RGBA8UNorm, MLSurfaceFormat_D32Float};
  MLHandle opengl_context = reinterpret_cast<MLHandle>(windowsystem::GetGLContext(window));
  mlContext->graphics_client = ML_INVALID_HANDLE;
  if (MLGraphicsCreateClientGL(&graphics_options, opengl_context, &mlContext->graphics_client) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to create graphics clent.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  MLGraphicsRenderTargetsInfo renderTargetsInfo;
  if (MLGraphicsGetRenderTargets(mlContext->graphics_client, &renderTargetsInfo) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to get graphics render targets.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  if (renderTargetsInfo.num_virtual_cameras != 2) {
    ML_LOG(Error, "Invalid graphics render targets num cameras: %u", renderTargetsInfo.num_virtual_cameras);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  /* for (int i = 0; i < 2; i++) {
    const MLGraphicsRenderBufferInfo &buffer = renderTargetsInfo.buffers[i];

    const MLGraphicsRenderTarget &colorRenderTarget = buffer.color;
    const uint32_t &colorWidth = colorRenderTarget.width;
    const uint32_t &colorHeight = colorRenderTarget.height;

    const MLGraphicsRenderTarget &depthRenderTarget = buffer.depth;
    const uint32_t &depthWidth = depthRenderTarget.width;
    const uint32_t &depthHeight = depthRenderTarget.height;

    ML_LOG(Info, "Got ml render target %u: %u x %u, %u x %u", i, colorWidth, colorHeight, depthWidth, depthHeight);
  } */

  /* MLResult privilege_init_status = MLPrivilegesStartup();
  if (privilege_init_status != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to initialize privilege system.", application_name);
    info.GetReturnValue().Set(JS_BOOL(false));
    return;
  } */

  // initialize perception system
  MLPerceptionSettings perception_settings;

  if (MLPerceptionInitSettings(&perception_settings) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to initialize perception.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  if (MLPerceptionStartup(&perception_settings) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to startup perception.", application_name);
    info.GetReturnValue().Set(JS_BOOL(false));
    return;
  }

  if (MLHeadTrackingCreate(&mlContext->head_tracker) != MLResult_Ok || MLHeadTrackingGetStaticData(mlContext->head_tracker, &mlContext->head_static_data) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to create head tracker.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  MLInputConfiguration inputConfiguration;
  for (int i = 0; i < MLInput_MaxControllers; i++) {
    inputConfiguration.dof[i] = MLInputControllerDof_6;
  }
  if (MLInputCreate(&inputConfiguration, &mlContext->inputTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to create input tracker.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  {
    keyboardCallbacks.on_char = onChar;
    keyboardCallbacks.on_key_down = onKeyDown;
    keyboardCallbacks.on_key_up = onKeyUp;
    MLResult result = MLInputSetKeyboardCallbacks(mlContext->inputTracker, &keyboardCallbacks, nullptr);
    if (result != MLResult_Ok) {
      ML_LOG(Error, "%s: failed to set keyboard callbacks %x", application_name, result);
      Nan::ThrowError("MLContext::InitLifecycle failed to set keyboard callbacks");
      return;
    }
  }

  /* if (MLGestureTrackingCreate(&mlContext->gestureTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to create gesture tracker.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  } */

  if (MLHandTrackingCreate(&handTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to create hand tracker.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  MLHandTrackingConfiguration handTrackingConfig;
  handTrackingConfig.handtracking_pipeline_enabled = true;
  handTrackingConfig.keypoints_filter_level = MLKeypointFilterLevel_0;
  handTrackingConfig.pose_filter_level = MLPoseFilterLevel_0;
  for (int i = 0; i < MLHandTrackingKeyPose_Count; i++) {
    handTrackingConfig.keypose_config[i] = true;
  }
  if (MLHandTrackingSetConfiguration(handTracker, &handTrackingConfig) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to set hand tracker config.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  MLMeshingSettings meshingSettings;
  if (MLMeshingInitSettings(&meshingSettings) != MLResult_Ok) {
    ML_LOG(Error, "%s: failed to initialize meshing settings", application_name);
  }
  meshingSettings.flags |= MLMeshingFlags_ComputeNormals;
  // meshingSettings.flags |= MLMeshingFlags_Planarize;
  {
    MLResult result = MLMeshingCreateClient(&meshTracker, &meshingSettings);
    if (result != MLResult_Ok) {
      ML_LOG(Error, "%s: failed to create mesh handle %x", application_name, result);
      Nan::ThrowError("MLContext::InitLifecycle failed to create mesh handle");
      return;
    }
  }

  if (MLPlanesCreate(&planesTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: failed to create planes handle", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  if (MLEyeTrackingCreate(&eyeTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: failed to create eye handle", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  // Now that graphics is connected, the app is ready to go
  if (MLLifecycleSetReadyIndication() != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to indicate lifecycle ready.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  // HACK: force the app to be "running"
  application_context.dummy_value = DummyValue::RUNNING;

  ML_LOG(Info, "%s: Start loop.", application_name);

  glGenFramebuffers(1, &mlContext->framebuffer_id);

  /* makePlanesQueryer(mlContext->planesFloorHandle);
  makePlanesQueryer(mlContext->planesWallHandle);
  makePlanesQueryer(mlContext->planesCeilingHandle);

  std::thread([mlContext]() {
    MLMeshingSettings meshingSettings;
    // meshingSettings.bounds_center = mlContext->position;
    // meshingSettings.bounds_rotation = mlContext->rotation;
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
    // meshingSettings.mesh_type = MLMeshingType_PointCloud;
    meshingSettings.mesh_type = MLMeshingType_Blocks;
    // meshingSettings.meshing_poll_time = 1e9;
    meshingSettings.meshing_poll_time = 0;
    meshingSettings.planarize = false;
    meshingSettings.remove_disconnected_components = false;
    meshingSettings.remove_mesh_skirt = false;
    meshingSettings.request_vertex_confidence = false;
    meshingSettings.target_number_triangles_per_block = 0;
    if (MLMeshingCreate(&meshingSettings, &mlContext->meshTracker) != MLResult_Ok) {
      ML_LOG(Error, "%s: Failed to create mesh handle.", application_name);
    }

    MLDataArrayInitDiff(&mlContext->meshesDataDiff);
    MLDataArrayInitDiff(&mlContext->meshesDataDiff2);

    std::mutex mesherCvMutex;
    for (;;) {
      std::unique_lock<std::mutex> uniqueLock(mesherCvMutex);
      mlContext->mesherCv.wait(uniqueLock);

      {
        std::unique_lock<std::mutex> uniqueLock(mlContext->positionMutex);

        meshingSettings.bounds_center = mlContext->position;
        meshingSettings.bounds_rotation = mlContext->rotation;
      }

      MLResult meshingUpdateResult = MLMeshingUpdate(mlContext->meshTracker, &meshingSettings);
      if (meshingUpdateResult != MLResult_Ok) {
        ML_LOG(Error, "MLMeshingUpdate failed: %s", application_name);
      }

      MLResult meshingStaticDataResult = MLMeshingGetStaticData(mlContext->meshTracker, &mlContext->meshStaticData);
      if (meshingStaticDataResult == MLResult_Ok) {
        std::unique_lock<std::mutex> uniqueLock(mlContext->mesherMutex);
        mlContext->haveMeshStaticData = true;
      } else {
        ML_LOG(Error, "MLMeshingGetStaticData failed: %s", application_name);
      }
    }
  });

  if (MLOcclusionCreateClient(&mlContext->occlusionTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to create occlusion tracker.", application_name);
  } */

  unsigned int width = renderTargetsInfo.buffers[0].color.width;
  unsigned int height = renderTargetsInfo.buffers[0].color.height;
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"), JS_INT(width));
  result->Set(JS_STR("height"), JS_INT(height));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(MLContext::Exit) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  if (MLGraphicsDestroyClient(&mlContext->graphics_client) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to create graphics clent.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  // destroy perception system
  if (MLHeadTrackingDestroy(mlContext->head_tracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to destroy head tracker.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  if (MLInputDestroy(mlContext->inputTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to destroy input tracker.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  if (MLHandTrackingDestroy(handTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to destroy hand tracker.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  if (MLMeshingDestroyClient(&meshTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: failed to destroy mesh handle", application_name);
    return;
  }

  if (MLPlanesDestroy(planesTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: failed to destroy planes handle", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  if (MLEyeTrackingDestroy(eyeTracker) != MLResult_Ok) {
    ML_LOG(Error, "%s: failed to create eye handle", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  if (MLPerceptionShutdown() != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to stop perception.", application_name);
    info.GetReturnValue().Set(JS_BOOL(false));
    return;
  }

  // HACK: force the app to be "stopped"
  application_context.dummy_value = DummyValue::STOPPED;

  glDeleteFramebuffers(1, &mlContext->framebuffer_id);
}

NAN_METHOD(MLContext::WaitGetPoses) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  if (info[0]->IsUint32Array() && info[1]->IsFloat32Array() && info[2]->IsFloat32Array() && info[3]->IsUint32Array() && info[4]->IsFloat32Array()) {
    if (application_context.dummy_value == DummyValue::RUNNING) {
      // Local<Uint32Array> framebufferArray = Local<Uint32Array>::Cast(info[0]);
      Local<Float32Array> transformArray = Local<Float32Array>::Cast(info[0]);
      Local<Float32Array> projectionArray = Local<Float32Array>::Cast(info[1]);
      // Local<Uint32Array> viewportArray = Local<Uint32Array>::Cast(info[2]);
      Local<Float32Array> controllersArray = Local<Float32Array>::Cast(info[2]);

      MLGraphicsFrameParams frame_params;
      MLResult result = MLGraphicsInitFrameParams(&frame_params);
      if (result != MLResult_Ok) {
        ML_LOG(Error, "MLGraphicsInitFrameParams complained: %d", result);
      }
      frame_params.surface_scale = 1.0f;
      frame_params.projection_type = MLGraphicsProjectionType_Default;
      frame_params.near_clip = 0.1f;
      frame_params.far_clip = 100.0f;
      frame_params.focus_distance = 1.0f;

      result = MLGraphicsBeginFrame(mlContext->graphics_client, &frame_params, &mlContext->frame_handle, &mlContext->virtual_camera_array);

      if (result == MLResult_Ok) {
        // framebuffer
        // framebufferArray->Set(0, JS_INT((unsigned int)mlContext->virtual_camera_array.color_id));
        // framebufferArray->Set(1, JS_INT((unsigned int)mlContext->virtual_camera_array.depth_id));

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

        // position
        {
          // std::unique_lock<std::mutex> lock(mlContext->positionMutex);

          const MLTransform &leftCameraTransform = mlContext->virtual_camera_array.virtual_cameras[0].transform;
          mlContext->position = leftCameraTransform.position;
          mlContext->rotation = leftCameraTransform.rotation;
        }

        /* // viewport
        const MLRectf &viewport = mlContext->virtual_camera_array.viewport;
        viewportArray->Set(0, JS_INT((int)viewport.x));
        viewportArray->Set(1, JS_INT((int)viewport.y));
        viewportArray->Set(2, JS_INT((unsigned int)viewport.w));
        viewportArray->Set(3, JS_INT((unsigned int)viewport.h)); */

        // controllers
        MLInputControllerState controllerStates[MLInput_MaxControllers];
        result = MLInputGetControllerState(mlContext->inputTracker, controllerStates);
        if (result == MLResult_Ok) {
          for (int i = 0; i < 2 && i < MLInput_MaxControllers; i++) {
            MLInputControllerState &controllerState = controllerStates[i];
            MLVec3f &position = controllerState.position;
            MLQuaternionf &orientation = controllerState.orientation;
            float trigger = controllerState.trigger_normalized;
            MLVec3f &touchPosAndForce = controllerState.touch_pos_and_force[0];

            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 0, JS_NUM(position.x));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 1, JS_NUM(position.y));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 2, JS_NUM(position.z));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 3, JS_NUM(orientation.x));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 4, JS_NUM(orientation.y));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 5, JS_NUM(orientation.z));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 6, JS_NUM(orientation.w));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 7, JS_NUM(trigger));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 8, JS_NUM(touchPosAndForce.x));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 9, JS_NUM(touchPosAndForce.y));
          }
        } else {
          ML_LOG(Error, "MLInputGetControllerState failed: %s", application_name);
        }

        /* // gestures
        MLGestureData gestureData;
        result = MLGestureGetData(mlContext->gestureTracker, &gestureData);
        if (result == MLResult_Ok) {
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
        } */

        // meshing
        /* {
          std::unique_lock<std::mutex> uniqueLock(mlContext->mesherMutex);

          if (mlContext->haveMeshStaticData) {
            // MLCoordinateFrameUID coordinateFrame = mlContext->meshStaticData.frame;
            MLDataArrayHandle &meshesHandle = mlContext->meshStaticData.meshes;

            MLResult lockResult = MLDataArrayTryLock(meshesHandle, &mlContext->meshData, &mlContext->meshesDataDiff);
            if (lockResult == MLResult_Ok) {
              if (mlContext->meshData.stream_count > 0) {
                MLDataArrayStream &handleStream = mlContext->meshData.streams[0];

                if (handleStream.type == MLDataArrayType_Handle) {
                  MLDataArrayHandle &meshesHandle2 = *handleStream.handle_array;

                  MLResult lockResult2 = MLDataArrayTryLock(meshesHandle2, &mlContext->meshData2, &mlContext->meshesDataDiff2);
                  if (lockResult2 == MLResult_Ok) {
                    uint32_t positionIndex = mlContext->meshStaticData.position_stream_index;
                    MLDataArrayStream &positionStream = mlContext->meshData2.streams[positionIndex];
                    uint32_t numPositionPoints = positionStream.count;
                    uint32_t positionsSize = numPositionPoints * positionStream.data_size;
                    mlContext->positions.resize(positionsSize);
                    memcpy(mlContext->positions.data(), positionStream.custom_array, positionsSize);

                    uint32_t normalIndex = mlContext->meshStaticData.normal_stream_index;
                    MLDataArrayStream &normalStream = mlContext->meshData2.streams[normalIndex];
                    uint32_t numNormalPoints = normalStream.count;
                    uint32_t normalsSize = numNormalPoints * normalStream.data_size;
                    mlContext->normals.resize(normalsSize);
                    memcpy(mlContext->normals.data(), normalStream.custom_array, normalsSize);

                    uint32_t triangleIndex = mlContext->meshStaticData.triangle_index_stream_index;
                    MLDataArrayStream &triangleStream = mlContext->meshData2.streams[triangleIndex];
                    uint32_t numTriangles = triangleStream.count;
                    uint32_t trianglesSize = numTriangles * triangleStream.data_size;
                    mlContext->triangles.resize(trianglesSize);
                    memcpy(mlContext->triangles.data(), triangleStream.custom_array, trianglesSize);

                    MLDataArrayUnlock(meshesHandle2);
                  } else if (lockResult2 == MLResult_Pending) {
                    // nothing
                  } else if (lockResult2 == MLResult_Locked) {
                    ML_LOG(Error, "MLDataArrayTryLock inner already locked: %d", lockResult2);
                  } else {
                    ML_LOG(Error, "MLDataArrayTryLock inner failed: %d", lockResult2);
                  }
                } else {
                  ML_LOG(Error, "invalid handle stream type: %d", handleStream.type);
                }
              } else {
                ML_LOG(Error, "invalid stream count: %d", mlContext->meshData.stream_count);
              }

              MLDataArrayUnlock(meshesHandle);
            } else if (lockResult == MLResult_Pending) {
              // nothing
            } else if (lockResult == MLResult_Locked) {
              ML_LOG(Error, "MLDataArrayTryLock outer already locked: %d", lockResult);
            } else {
              ML_LOG(Error, "MLDataArrayTryLock outer failed: %d", lockResult);
            }

            mlContext->haveMeshStaticData = false;
          }
        } */

        /* // occlusion
        MLOcclusionDepthBufferInfo occlusionBuffer;
        for (size_t i = 0; i < 2; i++) {
          occlusionBuffer.buffers[i].projection = mlContext->virtual_camera_array.virtual_cameras[i].projection;
          occlusionBuffer.buffers[i].transform = mlContext->virtual_camera_array.virtual_cameras[i].transform;
        }
        occlusionBuffer.projection_type = MLGraphicsProjectionType_Default;
        occlusionBuffer.num_buffers = 2;
        occlusionBuffer.viewport = viewport;
        MLResult result = MLOcclusionPopulateDepth(mlContext->occlusionTracker, &occlusionBuffer);
        if (result != MLResult_Ok) {
          ML_LOG(Error, "MLOcclusionPopulateDepth outer failed: %d", result);
        } */

        info.GetReturnValue().Set(JS_BOOL(true));
      } else {
        ML_LOG(Error, "MLGraphicsBeginFrame complained: %d", result);

        info.GetReturnValue().Set(JS_BOOL(false));
      }
    } else if (application_context.dummy_value == DummyValue::RUNNING) {
      Nan::ThrowError("MLContext::WaitGetPoses called for paused app");
    } else {
      Nan::ThrowError("MLContext::WaitGetPoses called for dead app");
    }
  } else {
    Nan::ThrowError("MLContext::WaitGetPoses: invalid arguments");
  }
}

NAN_METHOD(MLContext::SubmitFrame) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  // blit
  GLuint src_framebuffer_id = info[0]->Uint32Value();
  unsigned int width = info[1]->Uint32Value();
  unsigned int height = info[2]->Uint32Value();

  const MLRectf &viewport = mlContext->virtual_camera_array.viewport;

  for (int i = 0; i < 2; i++) {
    MLGraphicsVirtualCameraInfo &camera = mlContext->virtual_camera_array.virtual_cameras[i];

    glBindFramebuffer(GL_FRAMEBUFFER, mlContext->framebuffer_id);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mlContext->virtual_camera_array.color_id, 0, i);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mlContext->virtual_camera_array.depth_id, 0, i);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, src_framebuffer_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->framebuffer_id);

    glBlitFramebuffer(i == 0 ? 0 : width/2, 0,
      i == 0 ? width/2 : width, height,
      viewport.x, viewport.y,
      viewport.w, viewport.h,
      GL_COLOR_BUFFER_BIT,
      GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    MLResult result = MLGraphicsSignalSyncObjectGL(mlContext->graphics_client, camera.sync_object);
    if (result != MLResult_Ok) {
      ML_LOG(Error, "MLGraphicsSignalSyncObjectGL complained: %d", result);
    }
  }

  MLResult result = MLGraphicsEndFrame(mlContext->graphics_client, mlContext->frame_handle);
  if (result != MLResult_Ok) {
    ML_LOG(Error, "MLGraphicsEndFrame complained: %d", result);
  }

  // planes
  /* beginPlanesQuery(mlContext->position, mlContext->rotation, mlContext->planesFloorHandle, mlContext->planesFloorQueryHandle, static_cast<MLPlanesQueryFlags>(MLPlanesQueryFlag_AllOrientations | MLPlanesQueryFlag_Semantic_Floor));
  beginPlanesQuery(mlContext->position, mlContext->rotation, mlContext->planesWallHandle, mlContext->planesWallQueryHandle, static_cast<MLPlanesQueryFlags>(MLPlanesQueryFlag_AllOrientations | MLPlanesQueryFlag_Semantic_Wall));
  beginPlanesQuery(mlContext->position, mlContext->rotation, mlContext->planesCeilingHandle, mlContext->planesCeilingQueryHandle, static_cast<MLPlanesQueryFlags>(MLPlanesQueryFlag_AllOrientations | MLPlanesQueryFlag_Semantic_Ceiling));

  // meshing
  mlContext->mesherCv.notify_one(); */
}

NAN_METHOD(MLContext::IsPresent) {
  info.GetReturnValue().Set(JS_BOOL(isPresent()));
}

NAN_METHOD(MLContext::IsSimulated) {
  info.GetReturnValue().Set(JS_BOOL(isSimulated()));
}

NAN_METHOD(MLContext::OnPresentChange) {
  if (info[0]->IsFunction()) {
    Local<Function> initCbFn = Local<Function>::Cast(info[0]);
    initCb.Reset(initCbFn);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::RequestHand) {
  if (info[0]->IsFunction()) {
    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    HandRequest *handRequest = new HandRequest(cbFn);
    handRequests.push_back(handRequest);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::CancelHand) {
  if (info[0]->IsFunction()) {
    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    {
      handRequests.erase(std::remove_if(handRequests.begin(), handRequests.end(), [&](HandRequest *h) -> bool {
        Local<Function> localCbFn = Nan::New(h->cbFn);
        if (localCbFn->StrictEquals(cbFn)) {
          delete h;
          return false;
        } else {
          return true;
        }
      }));
    }
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::RequestMesh) {
  if (info[0]->IsFunction()) {
    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    MeshRequest *meshRequest = new MeshRequest(cbFn);
    meshRequests.push_back(meshRequest);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::CancelMesh) {
  if (info[0]->IsFunction()) {
    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    {
      meshRequests.erase(std::remove_if(meshRequests.begin(), meshRequests.end(), [&](MeshRequest *m) -> bool {
        Local<Function> localCbFn = Nan::New(m->cbFn);
        if (localCbFn->StrictEquals(cbFn)) {
          delete m;
          return false;
        } else {
          return true;
        }
      }));
    }
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::RequestPlanes) {
  if (info[0]->IsFunction()) {
    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    PlanesRequest *planesRequest = new PlanesRequest(cbFn);
    planesRequests.push_back(planesRequest);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::CancelPlanes) {
  if (info[0]->IsFunction()) {
    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    {
      planesRequests.erase(std::remove_if(planesRequests.begin(), planesRequests.end(), [&](PlanesRequest *p) -> bool {
        Local<Function> localCbFn = Nan::New(p->cbFn);
        if (localCbFn->StrictEquals(cbFn)) {
          delete p;
          return false;
        } else {
          return true;
        }
      }));
    }
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::RequestEye) {
  if (info[0]->IsFunction()) {
    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    EyeRequest *eyeRequest = new EyeRequest(cbFn);
    eyeRequests.push_back(eyeRequest);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::CancelEye) {
  if (info[0]->IsFunction()) {
    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    {
      eyeRequests.erase(std::remove_if(eyeRequests.begin(), eyeRequests.end(), [&](EyeRequest *e) -> bool {
        Local<Function> localCbFn = Nan::New(e->cbFn);
        if (localCbFn->StrictEquals(cbFn)) {
          delete e;
          return false;
        } else {
          return true;
        }
      }));
    }
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

bool cameraConnected = false;
NAN_METHOD(MLContext::RequestCamera) {
  if (info[0]->IsFunction()) {
    if (!cameraConnected) {
      MLResult result = MLCameraConnect();
      if (result == MLResult_Ok) {
        cameraRequestThread = std::thread([&]() -> void {
          for (;;) {
            std::unique_lock<std::mutex> lock(cameraRequestMutex);
            cameraRequestConditionVariable.wait(lock);

            MLResult result = MLCameraCaptureImageRaw();
            if (result == MLResult_Ok) {
              // nothing
            } else {
              ML_LOG(Error, "%s: Failed to capture image: %x", application_name, result);
            }
          }
        });
        cameraRequestThread.detach();

        cameraConnected = true;
      } else {
        ML_LOG(Error, "%s: Failed to connect camera: %x", application_name, result);
        Nan::ThrowError("failed to connect camera");
        return;
      }
    }

    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    MLResult result = MLCameraSetOutputFormat(MLCameraOutputFormat_YUV_420_888);
    if (result == MLResult_Ok) {
      MLHandle captureHandle;
      MLResult result = MLCameraPrepareCapture(MLCameraCaptureType_ImageRaw, &captureHandle);
      if (result == MLResult_Ok) {
        {
          std::unique_lock<std::mutex> lock(cameraRequestsMutex);
          CameraRequest *cameraRequest = new CameraRequest(captureHandle, cbFn);
          cameraRequests.push_back(cameraRequest);
        }

        /* MLResult result = MLCameraCaptureImageRaw();
        if (result == MLResult_Ok) {
          // XXX
        } else {
          ML_LOG(Error, "%s: Failed to capture image: %x", application_name, result);
          Nan::ThrowError("failed to capture image");
          return;
        } */
      } else {
        ML_LOG(Error, "%s: Failed to prepare camera capture: %x", application_name, result);
        Nan::ThrowError("failed to prepare camera capture");
        return;
      }
    } else {
      ML_LOG(Error, "%s: Failed to set camera output format: %x", application_name, result);
      Nan::ThrowError("failed to prepare camera capture");
      return;
    }
  } else {
    Nan::ThrowError("invalid arguments");
    return;
  }
}

NAN_METHOD(MLContext::CancelCamera) {
  if (info[0]->IsFunction()) {
    Local<Function> cbFn = Local<Function>::Cast(info[0]);

    {
      std::unique_lock<std::mutex> lock(cameraRequestsMutex);
      cameraRequests.erase(std::remove_if(cameraRequests.begin(), cameraRequests.end(), [&](CameraRequest *c) -> bool {
        Local<Function> localCbFn = Nan::New(c->cbFn);
        if (localCbFn->StrictEquals(cbFn)) {
          delete c;
          return false;
        } else {
          return true;
        }
      }));
    }
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::PrePollEvents) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(Local<Object>::Cast(info[0]));

  if (handRequests.size() > 0) {
    MLResult result = MLHandTrackingGetData(handTracker, &handData);
    if (result == MLResult_Ok) {
      std::for_each(handRequests.begin(), handRequests.end(), [&](HandRequest *h) {
        h->Poll();
      });
    } else {
      ML_LOG(Error, "%s: Hand data get failed! %x", application_name, result);
    }
  }

  if (meshRequests.size() > 0 && !meshInfoRequestPending && !meshRequestsPending) {
    {
      // std::unique_lock<std::mutex> lock(mlContext->positionMutex);

      meshExtents.center = mlContext->position;
      // meshExtents.rotation =  mlContext->rotation;
      meshExtents.rotation = {0, 0, 0, 1};
    }
    meshExtents.extents.x = 3;
    meshExtents.extents.y = 3;
    meshExtents.extents.z = 3;

    MLResult result = MLMeshingRequestMeshInfo(meshTracker, &meshExtents, &meshInfoRequestHandle);
    if (result == MLResult_Ok) {
      meshInfoRequestPending = true;
    } else {
      ML_LOG(Error, "%s: Mesh info request failed! %x", application_name, result);
    }
  }

  if (planesRequests.size() > 0 && !planesRequestPending) {
    {
      // std::unique_lock<std::mutex> lock(mlContext->positionMutex);

      planesRequest.bounds_center = mlContext->position;
      // planesRequest.bounds_rotation = mlContext->rotation;
      planesRequest.bounds_rotation = {0, 0, 0, 1};
    }
    planesRequest.bounds_extents.x = 3;
    planesRequest.bounds_extents.y = 3;
    planesRequest.bounds_extents.z = 3;

    planesRequest.flags = MLPlanesQueryFlag_Arbitrary | MLPlanesQueryFlag_AllOrientations | MLPlanesQueryFlag_Semantic_All;
    planesRequest.min_hole_length = 0.5;
    planesRequest.min_plane_area = 0.25;
    planesRequest.max_results = MAX_NUM_PLANES;

    MLResult result = MLPlanesQueryBegin(planesTracker, &planesRequest, &planesRequestHandle);
    if (result == MLResult_Ok) {
      planesRequestPending = true;
    } else {
      ML_LOG(Error, "%s: Planes request failed! %x", application_name, result);
    }
  }

  if (eyeRequests.size() > 0) {
    if (MLEyeTrackingGetState(eyeTracker, &eyeState) != MLResult_Ok) {
      ML_LOG(Error, "%s: Eye get state failed!", application_name);
    }

    if (MLEyeTrackingGetStaticData(eyeTracker, &eyeStaticData) != MLResult_Ok) {
      ML_LOG(Error, "%s: Eye get static data failed!", application_name);
    }

    std::for_each(eyeRequests.begin(), eyeRequests.end(), [&](EyeRequest *e) {
      e->Poll();
    });
  }

  {
    std::unique_lock<std::mutex> lock(cameraRequestsMutex);
    if (cameraRequests.size() > 0) {
      cameraRequestConditionVariable.notify_one();
    }
  }
}

NAN_METHOD(MLContext::PostPollEvents) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(Local<Object>::Cast(info[0]));
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[1]));
  GLuint fbo = info[2]->Uint32Value();
  unsigned int width = info[3]->Uint32Value();
  unsigned int height = info[4]->Uint32Value();

  if (meshInfoRequestPending) {
    MLResult result = MLMeshingGetMeshInfoResult(meshTracker, meshInfoRequestHandle, &meshInfo);
    if (result == MLResult_Ok) {
      uint32_t dataCount = meshInfo.data_count;
      uint32_t requestCount = 0;
      meshRemovedList.clear();
      for (uint32_t i = 0; i < dataCount; i++) {
        const MLMeshingBlockInfo &meshBlockInfo = meshInfo.data[i];
        const MLMeshingMeshState &state = meshBlockInfo.state;
        if (state == MLMeshingMeshState_New || state == MLMeshingMeshState_Updated) {
          MLMeshingBlockRequest &meshBlockRequest = meshBlockRequests[requestCount++];
          meshBlockRequest.id = meshBlockInfo.id;
          // meshBlockRequest.level = MLMeshingLOD_Minimum;
          meshBlockRequest.level = MLMeshingLOD_Medium;
          // meshBlockRequest.level = MLMeshingLOD_Maximum;
        } else if (state == MLMeshingMeshState_Deleted) {
          meshRemovedList.push_back(meshBlockInfo.id);
        }
      }
      numMeshBlockRequests = requestCount;

      meshInfoRequestPending = false;
      meshRequestsPending = true;
      meshRequestPending = false;
      meshBlockRequestIndex = 0;
      MLMeshingFreeResource(meshTracker, &meshInfoRequestHandle);
    } else if (result == MLResult_Pending) {
      // nothing
    } else {
      ML_LOG(Error, "%s: Mesh info get failed! %x", application_name, result);

      meshInfoRequestPending = false;
    }
  }
  if (meshRequestsPending && !meshRequestPending) {
    if (meshBlockRequestIndex < numMeshBlockRequests) {
      uint32_t requestsRemaining = numMeshBlockRequests - meshBlockRequestIndex;
      uint32_t requestsThisTime = std::min<uint32_t>(requestsRemaining, 16);
      meshRequest.data = meshBlockRequests.data() + meshBlockRequestIndex;
      meshRequest.request_count = requestsThisTime;

      meshBlockRequestIndex += requestsThisTime;

      MLResult result = MLMeshingRequestMesh(meshTracker, &meshRequest, &meshRequestHandle);
      if (result == MLResult_Ok) {
        meshRequestsPending = true;
        meshRequestPending = true;
      } else {
        ML_LOG(Error, "%s: Mesh request failed! %x", application_name, result);

        meshRequestsPending = false;
        meshRequestPending = false;
      }
    } else {
      meshRequestsPending = false;
      meshRequestPending = false;
    }
  }
  if (meshRequestsPending && meshRequestPending) {
    MLResult result = MLMeshingGetMeshResult(meshTracker, meshRequestHandle, &mesh);
    if (result == MLResult_Ok) {
      std::for_each(meshRequests.begin(), meshRequests.end(), [&](MeshRequest *m) {
        m->Poll();
      });
      if (meshRemovedList.size() > 0) {
        meshRemovedList.clear();
      }
      if (gl->HasBufferBinding(GL_ARRAY_BUFFER)) {
        glBindBuffer(GL_ARRAY_BUFFER, gl->GetBufferBinding(GL_ARRAY_BUFFER));
      }

      meshRequestsPending = true;
      meshRequestPending = false;
      MLMeshingFreeResource(meshTracker, &meshRequestHandle);
    } else if (result == MLResult_Pending) {
      // nothing
    } else {
      ML_LOG(Error, "%s: Mesh get failed! %x", application_name, result);

      meshRequestsPending = true;
      meshRequestPending = false;
    }
  }

  if (planesRequestPending) {
    MLResult result = MLPlanesQueryGetResults(planesTracker, planesRequestHandle, planeResults, &numPlanesResults);
    if (result == MLResult_Ok) {
      std::for_each(planesRequests.begin(), planesRequests.end(), [&](PlanesRequest *p) {
        p->Poll();
      });

      planesRequestPending = false;
    } else if (result == MLResult_Pending) {
      // nothing
    } else {
      ML_LOG(Error, "%s: Planes request failed! %x", application_name, result);

      planesRequestPending = false;
    }
  }

  {
    std::unique_lock<std::mutex> lock(cameraRequestsMutex);

    if (cameraRequests.size() > 0 && cameraResponsePending) {
      std::for_each(cameraRequests.begin(), cameraRequests.end(), [&](CameraRequest *c) {
        c->Poll(gl, fbo, width, height);
      });
      cameraResponsePending = false;
    }
  }
}

}

Handle<Object> makeMl() {
  Nan::EscapableHandleScope scope;
  return scope.Escape(ml::MLContext::Initialize(Isolate::GetCurrent()));
}

#endif

#if defined(LUMIN)

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

Nan::Persistent<Function> mlMesherConstructor;
Nan::Persistent<Function> mlPlaneTrackerConstructor;
Nan::Persistent<Function> mlHandTrackerConstructor;
Nan::Persistent<Function> mlEyeTrackerConstructor;

std::thread cameraRequestThread;
std::mutex cameraRequestMutex;
std::condition_variable  cameraRequestConditionVariable;
std::mutex cameraRequestsMutex;
std::vector<CameraRequest *> cameraRequests;
bool cameraResponsePending = false;

MLHandle handTracker;
MLHandTrackingData handData;
MLHandTrackingStaticData handStaticData;
std::vector<MLHandTracker *> handTrackers;
float wristBones[2][4][1 + 3];
float fingerBones[2][5][4][1 + 3];

MLHandle meshTracker;
std::vector<MLMesher *> meshers;
MLMeshingExtents meshExtents;
MLHandle meshInfoRequestHandle;
MLMeshingMeshInfo meshInfo;
bool meshInfoRequestPending = false;
std::vector<MLMeshingBlockRequest> meshBlockRequests(128);
uint32_t numMeshBlockRequests = 0;
uint32_t meshBlockRequestIndex = 0;
MLMeshingMeshRequest meshRequest;
std::map<std::string, bool> meshRequestNewMap;
std::vector<MLCoordinateFrameUID> meshRemovedList;
MLHandle meshRequestHandle;
MLMeshingMesh mesh;
bool meshRequestsPending = false;
bool meshRequestPending = false;

std::map<std::string, MeshBuffer> meshBuffers;

MLHandle planesTracker;
std::vector<MLPlaneTracker *> planeTrackers;
MLPlanesQuery planesRequest;
MLHandle planesRequestHandle;
MLPlane planeResults[MAX_NUM_PLANES];
uint32_t numPlanesResults;
bool planesRequestPending = false;

MLHandle eyeTracker;
std::vector<MLEyeTracker *> eyeTrackers;
MLEyeTrackingState eyeState;
MLEyeTrackingStaticData eyeStaticData;

bool depthEnabled = false;

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

std::string id2String(const MLCoordinateFrameUID &id) {
  uint64_t id1 = id.data[0];
  uint64_t id2 = id.data[1];
  char idbuf[16*2 + 1];
  sprintf(idbuf, "%016llx%016llx", id1, id2);
  return std::string(idbuf);
}
std::string id2String(const uint64_t &id) {
  char idbuf[16 + 1];
  sprintf(idbuf, "%016llx", id);
  return std::string(idbuf);
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
}
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
} */
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

// MeshBuffer

MeshBuffer::MeshBuffer(GLuint positionBuffer, GLuint normalBuffer, GLuint indexBuffer) :
  positionBuffer(positionBuffer),
  normalBuffer(normalBuffer),
  indexBuffer(indexBuffer),
  numPositions(0),
  numIndices(0),
  isNew(true)
  {}
MeshBuffer::MeshBuffer(const MeshBuffer &meshBuffer) {
  positionBuffer = meshBuffer.positionBuffer;
  normalBuffer = meshBuffer.normalBuffer;
  indexBuffer = meshBuffer.indexBuffer;
  numPositions = meshBuffer.numPositions;
  numIndices = meshBuffer.numIndices;
  isNew = meshBuffer.isNew;
}
MeshBuffer::MeshBuffer() : positionBuffer(0), normalBuffer(0), indexBuffer(0), numPositions(0), numIndices(0), isNew(true) {}

void MeshBuffer::setBuffers(float *positions, uint32_t numPositions, float *normals, uint16_t *indices, uint16_t numIndices, bool isNew) {
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(positions[0]), positions, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
  glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(normals[0]), normals, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
  glBufferData(GL_ARRAY_BUFFER, numIndices * sizeof(indices[0]), indices, GL_DYNAMIC_DRAW);

  this->numPositions = numPositions;
  this->numIndices = numIndices;
  this->isNew = isNew;
}

// MLMesher

MLMesher::MLMesher() {}

MLMesher::~MLMesher() {}

NAN_METHOD(MLMesher::New) {
  MLMesher *mlMesher = new MLMesher();
  Local<Object> mlMesherObj = info.This();
  mlMesher->Wrap(mlMesherObj);

  Nan::SetAccessor(mlMesherObj, JS_STR("onmesh"), OnMeshGetter, OnMeshSetter);

  info.GetReturnValue().Set(mlMesherObj);

  meshers.push_back(mlMesher);
}

Local<Function> MLMesher::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("MLMesher"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "destroy", Destroy);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_GETTER(MLMesher::OnMeshGetter) {
  // Nan::HandleScope scope;
  MLMesher *mlMesher = ObjectWrap::Unwrap<MLMesher>(info.This());

  Local<Function> cb = Nan::New(mlMesher->cb);
  info.GetReturnValue().Set(cb);
}

NAN_SETTER(MLMesher::OnMeshSetter) {
  // Nan::HandleScope scope;
  MLMesher *mlMesher = ObjectWrap::Unwrap<MLMesher>(info.This());

  if (value->IsFunction()) {
    Local<Function> localCb = Local<Function>::Cast(value);
    mlMesher->cb.Reset(localCb);
  } else {
    Nan::ThrowError("MLMesher::OnMeshSetter: invalid arguments");
  }
}

void MLMesher::Poll() {
  if (!this->cb.IsEmpty()) {
    Local<Object> asyncObject = Nan::New<Object>();
    AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "MLMesher::Poll");

    MLMeshingBlockMesh *blockMeshes = mesh.data;
    uint32_t dataCount = mesh.data_count;

    Local<Array> array = Nan::New<Array>();
    uint32_t numResults = 0;
    for (uint32_t i = 0; i < dataCount; i++) {
      MLMeshingBlockMesh &blockMesh = blockMeshes[i];

      const MLMeshingResult &result = blockMesh.result;
      if (result == MLMeshingResult_Success || result == MLMeshingResult_PartialUpdate) {
        const std::string &id = id2String(blockMesh.id);

        auto iter = meshBuffers.find(id);
        if (iter != meshBuffers.end()) {
          const MeshBuffer &meshBuffer = iter->second;

          Local<Object> obj = Nan::New<Object>();
          obj->Set(JS_STR("id"), JS_STR(id));
          obj->Set(JS_STR("type"), JS_STR(meshBuffer.isNew ? "new" : "update"));

          Local<Object> positionObj = Nan::New<Object>();
          positionObj->Set(JS_STR("id"), JS_INT(meshBuffer.positionBuffer));
          obj->Set(JS_STR("position"), positionObj);
          obj->Set(JS_STR("positionCount"), JS_INT(meshBuffer.numPositions));
          Local<Object> normalObj = Nan::New<Object>();
          normalObj->Set(JS_STR("id"), JS_INT(meshBuffer.normalBuffer));
          obj->Set(JS_STR("normal"), normalObj);
          obj->Set(JS_STR("normalCount"), JS_INT(meshBuffer.numPositions));
          Local<Object> indexObj = Nan::New<Object>();
          indexObj->Set(JS_STR("id"), JS_INT(meshBuffer.indexBuffer));
          obj->Set(JS_STR("index"), indexObj);
          obj->Set(JS_STR("count"), JS_INT(meshBuffer.numIndices));

          array->Set(numResults++, obj);
        } else {
          // ML_LOG(Error, "%s: ML mesh poll failed to find mesh: %s", application_name, idbuf);
        }
      } else if (result == MLMeshingResult_Pending) {
        // nothing
      } else {
        ML_LOG(Error, "%s: ML mesh request poll failed: %x %x", application_name, i, result);
      }
    }
    for (const MLCoordinateFrameUID &cfid : meshRemovedList) {
      const std::string &id = id2String(cfid);

      Local<Object> obj = Nan::New<Object>();
      obj->Set(JS_STR("id"), JS_STR(id));
      obj->Set(JS_STR("type"), JS_STR("remove"));

      array->Set(numResults++, obj);
    }

    Local<Function> cbFn = Nan::New(this->cb);
    Local<Value> argv[] = {
      array,
    };
    asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);
  }
}

NAN_METHOD(MLMesher::Destroy) {
  MLMesher *mlMesher = ObjectWrap::Unwrap<MLMesher>(info.This());

  meshers.erase(std::remove_if(meshers.begin(), meshers.end(), [&](MLMesher *m) -> bool {
    if (m == mlMesher) {
      delete m;
      return true;
    } else {
      return false;
    }
  }), meshers.end());
}

// MLPlaneTracker

MLPlaneTracker::MLPlaneTracker() {}

MLPlaneTracker::~MLPlaneTracker() {}

NAN_METHOD(MLPlaneTracker::New) {
  MLPlaneTracker *mlPlaneTracker = new MLPlaneTracker();
  Local<Object> mlPlaneTrackerObj = info.This();
  mlPlaneTracker->Wrap(mlPlaneTrackerObj);

  Nan::SetAccessor(mlPlaneTrackerObj, JS_STR("onplanes"), OnPlanesGetter, OnPlanesSetter);

  info.GetReturnValue().Set(mlPlaneTrackerObj);

  planeTrackers.push_back(mlPlaneTracker);
}

Local<Function> MLPlaneTracker::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("MLPlaneTracker"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "destroy", Destroy);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_GETTER(MLPlaneTracker::OnPlanesGetter) {
  // Nan::HandleScope scope;
  MLPlaneTracker *mlPlaneTracker = ObjectWrap::Unwrap<MLPlaneTracker>(info.This());

  Local<Function> cb = Nan::New(mlPlaneTracker->cb);
  info.GetReturnValue().Set(cb);
}

NAN_SETTER(MLPlaneTracker::OnPlanesSetter) {
  // Nan::HandleScope scope;
  MLPlaneTracker *mlPlaneTracker = ObjectWrap::Unwrap<MLPlaneTracker>(info.This());

  if (value->IsFunction()) {
    Local<Function> localCb = Local<Function>::Cast(value);
    mlPlaneTracker->cb.Reset(localCb);
  } else {
    Nan::ThrowError("MLPlaneTracker::OnPlaneSetter: invalid arguments");
  }
}

void MLPlaneTracker::Poll() {
  if (!this->cb.IsEmpty()) {
    Local<Object> asyncObject = Nan::New<Object>();
    AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "MLPlaneTracker::Poll");

    Local<Array> array = Nan::New<Array>(numPlanesResults);
    for (uint32_t i = 0; i < numPlanesResults; i++) {
      MLPlane &plane = planeResults[i];

      Local<Object> obj = Nan::New<Object>();

      uint64_t planeId = (uint64_t)plane.id;
      // uint32_t flags = plane.flags;
      float width = plane.width;
      float height = plane.height;
      MLVec3f &position = plane.position;
      MLQuaternionf &rotation = plane.rotation;

      const std::string &id = id2String(planeId);
      obj->Set(JS_STR("id"), JS_STR(id));

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

      Local<Float32Array> sizeArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 2 * sizeof(float)), 0, 2);
      sizeArray->Set(0, JS_NUM(width));
      sizeArray->Set(1, JS_NUM(height));
      obj->Set(JS_STR("size"), sizeArray);

      array->Set(i, obj);
    }

    Local<Function> cb = Nan::New(this->cb);
    Local<Value> argv[] = {
      array,
    };
    asyncResource.MakeCallback(cb, sizeof(argv)/sizeof(argv[0]), argv);
  }
}

NAN_METHOD(MLPlaneTracker::Destroy) {
  MLPlaneTracker *mlPlaneTracker = ObjectWrap::Unwrap<MLPlaneTracker>(info.This());

  planeTrackers.erase(std::remove_if(planeTrackers.begin(), planeTrackers.end(), [&](MLPlaneTracker *p) -> bool {
    if (p == mlPlaneTracker) {
      delete p;
      return true;
    } else {
      return false;
    }
  }), planeTrackers.end());
}

// MLHandTracker

MLHandTracker::MLHandTracker() {}

MLHandTracker::~MLHandTracker() {}

NAN_METHOD(MLHandTracker::New) {
  MLHandTracker *mlHandTracker = new MLHandTracker();
  Local<Object> mlHandTrackerObj = info.This();
  mlHandTracker->Wrap(mlHandTrackerObj);

  Nan::SetAccessor(mlHandTrackerObj, JS_STR("onhands"), OnHandsGetter, OnHandsSetter);

  info.GetReturnValue().Set(mlHandTrackerObj);

  handTrackers.push_back(mlHandTracker);
}

Local<Function> MLHandTracker::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("MLHandTracker"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "destroy", Destroy);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_GETTER(MLHandTracker::OnHandsGetter) {
  // Nan::HandleScope scope;
  MLHandTracker *mlHandTracker = ObjectWrap::Unwrap<MLHandTracker>(info.This());

  Local<Function> cb = Nan::New(mlHandTracker->cb);
  info.GetReturnValue().Set(cb);
}

NAN_SETTER(MLHandTracker::OnHandsSetter) {
  // Nan::HandleScope scope;
  MLHandTracker *mlHandTracker = ObjectWrap::Unwrap<MLHandTracker>(info.This());

  if (value->IsFunction()) {
    Local<Function> localCb = Local<Function>::Cast(value);
    mlHandTracker->cb.Reset(localCb);
  } else {
    Nan::ThrowError("MLHandTracker::OnHandsSetter: invalid arguments");
  }
}

inline MLVec3f subVectors(const MLVec3f &a, const MLVec3f &b) {
  return MLVec3f{
    a.x - b.x,
    a.y - b.y,
    a.z - b.z
  };
}

inline MLVec3f divideVector(const MLVec3f &v, float l) {
  return MLVec3f{
    v.x / l,
    v.y / l,
    v.z / l
  };
}

inline float dotVectors(const MLVec3f &a, const MLVec3f &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline MLVec3f crossVectors(const MLVec3f &a, const MLVec3f &b) {
	return MLVec3f{
    a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
  };
}

inline float vectorLength(const MLVec3f &v) {
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline MLVec3f normalizeVector(const MLVec3f &v) {
  return divideVector(v, vectorLength(v));
}

inline float quaternionLength(const MLQuaternionf &q) {
  return sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

inline MLQuaternionf normalizeQuaternion(const MLQuaternionf &q) {
  float l = quaternionLength(q);

  if (l == 0) {
    return MLQuaternionf{
      0,
      0,
      0,
      1
    };
  } else {
    l = 1 / l;
    return MLQuaternionf{
      q.x * l,
      q.y * l,
      q.z * l,
      q.w * l
    };
  }
}

inline MLQuaternionf getQuaternionFromUnitVectors(const MLVec3f &vFrom, const MLVec3f &vTo) {
  constexpr float EPS = 0.000001;

  MLVec3f v1;
  float r = dotVectors(vFrom, vTo) + 1;
  if (r < EPS) {
    r = 0;

    if (std::abs(vFrom.x) > std::abs(vFrom.z)) {
      v1 = MLVec3f{-vFrom.y, vFrom.x, 0};
    } else {
      v1 = MLVec3f{0, -vFrom.z, vFrom.y};
    }
  } else {
    v1 = crossVectors(vFrom, vTo);
  }

  MLQuaternionf result{
    v1.x,
    v1.y,
    v1.z,
    r
  };
  return normalizeQuaternion(result);
}

bool hasHandBone(int handIndex) {
  for (size_t i = 0; i < 4; i++) {
    if (*(uint32_t *)&wristBones[handIndex][i][0]) {
      return true;
    }
  }
  for (size_t i = 0; i < 5; i++) {
    for (size_t j = 0; j < 4; j++) {
      if (*(uint32_t *)&fingerBones[handIndex][i][j][0]) {
        return true;
      }
    }
  }
  return false;
}

bool getBonesTransform(MLTransform &transform, std::vector<std::vector<float *>> &fingers) {
  return std::any_of(fingers.begin(), fingers.end(), [&](std::vector<float *> &bones) -> bool {
    std::vector<float *> validBones(bones);
    validBones.erase(std::remove_if(validBones.begin(), validBones.end(), [&](float *bone) -> bool {
      if (*(uint32_t *)&bone[0]) {
        return false; // keep
      } else {
        return true; // remove
      }
    }), validBones.end());
    if (validBones.size() >= 2) {
      float *startBoneArray = validBones[0];
      const MLVec3f startBone = {
        startBoneArray[1],
        startBoneArray[2],
        startBoneArray[3]
      };
      float *endBoneArray = validBones[validBones.size() - 1];
      const MLVec3f endBone = {
        endBoneArray[1],
        endBoneArray[2],
        endBoneArray[3]
      };
      const MLVec3f &direction = normalizeVector(subVectors(endBone, startBone));
      const MLQuaternionf &rotation = getQuaternionFromUnitVectors(
        MLVec3f{0, 1, 0},
        direction
      );

      transform.position = endBone;
      transform.rotation = rotation;

      return true;
    } else {
      return false;
    }
  });
}

bool getHandPointerTransform(MLTransform &transform, float wristBones[4][1 + 3], float fingerBones[5][4][1 + 3]) {
  std::vector<std::vector<float *>> fingers = {
    { // index
      fingerBones[1][0],
      fingerBones[1][1],
      fingerBones[1][2],
      fingerBones[1][3],
    },
    { // middle
      fingerBones[2][0],
      fingerBones[2][1],
      fingerBones[2][2],
      fingerBones[2][3],
    },
    { // thumb
      fingerBones[0][0],
      fingerBones[0][1],
      fingerBones[0][2],
      fingerBones[0][3],
    },
  };
  return getBonesTransform(transform, fingers);
}

bool getHandGripTransform(MLTransform &transform, float wristBones[4][1 + 3], float fingerBones[5][4][1 + 3]) {
  std::vector<std::vector<float *>> fingers = {
    { // wrist center, middleFinger
      wristBones[0],
      fingerBones[2][0],
      fingerBones[2][1],
      fingerBones[2][2],
      fingerBones[2][3],
    },
    { // thumb
      fingerBones[0][0],
      fingerBones[0][1],
      fingerBones[0][2],
      fingerBones[0][3],
    },
  };
  return getBonesTransform(transform, fingers);
}

void MLHandTracker::Poll() {
  Local<Object> asyncObject = Nan::New<Object>();
  AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "MLHandTracker::Poll");

  Local<Array> array = Nan::New<Array>();
  uint32_t numResults = 0;

  MLTransform leftPointerTransform;
  MLTransform leftGripTransform;
  MLTransform rightPointerTransform;
  MLTransform rightGripTransform;

  if (hasHandBone(0)) {
    Local<Object> obj = Nan::New<Object>();

    obj->Set(JS_STR("hand"), JS_STR("left"));

    if (getHandPointerTransform(leftPointerTransform, wristBones[0], fingerBones[0])) {
      Local<Object> pointerObj = Nan::New<Object>();
      pointerObj->Set(JS_STR("position"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftPointerTransform.position.values, 3 * sizeof(float)), 0, 3));
      pointerObj->Set(JS_STR("rotation"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftPointerTransform.rotation.values, 4 * sizeof(float)), 0, 4));
      obj->Set(JS_STR("pointer"), pointerObj);
    } else {
      obj->Set(JS_STR("pointer"), Nan::Null());
    }
    if (getHandGripTransform(leftGripTransform, wristBones[0], fingerBones[0])) {
      Local<Object> gripObj = Nan::New<Object>();
      gripObj->Set(JS_STR("position"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftGripTransform.position.values, 3 * sizeof(float)), 0, 3));
      gripObj->Set(JS_STR("rotation"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftGripTransform.rotation.values, 4 * sizeof(float)), 0, 4));
      obj->Set(JS_STR("grip"), gripObj);
    } else {
      obj->Set(JS_STR("grip"), Nan::Null());
    }
    
    Local<Array> wristArray = Nan::New<Array>(4);
    for (size_t i = 0; i < 4; i++) {
      Local<Value> boneVal;

      if (*(uint32_t *)&wristBones[0][i][0]) {
        boneVal = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)&wristBones[0][i][1], 3 * sizeof(float)), 0, 3);
      } else {
        boneVal = Nan::Null();
      }

      wristArray->Set(i, boneVal);
    }
    obj->Set(JS_STR("wrist"), wristArray);

    Local<Array> fingersArray = Nan::New<Array>(5);
    for (size_t i = 0; i < 5; i++) {
      Local<Array> bonesArray = Nan::New<Array>(4);

      for (size_t j = 0; j < 4; j++) {
        Local<Value> boneVal;

        if (*(uint32_t *)&fingerBones[0][i][j][0]) {
          boneVal = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)&fingerBones[0][i][j][1], 3 * sizeof(float)), 0, 3);
        } else {
          boneVal = Nan::Null();
        }

        bonesArray->Set(j, boneVal);
      }

      fingersArray->Set(i, bonesArray);
    }
    obj->Set(JS_STR("fingers"), fingersArray);

    const char *gesture = gestureCategoryToDescriptor(handData.left_hand_state.keypose);
    if (gesture) {
      obj->Set(JS_STR("gesture"), JS_STR(gesture));
    } else {
      obj->Set(JS_STR("gesture"), Nan::Null());
    }

    array->Set(JS_INT(numResults++), obj);
  }

  if (hasHandBone(1)) {
    Local<Object> obj = Nan::New<Object>();

    obj->Set(JS_STR("hand"), JS_STR("right"));

    if (getHandPointerTransform(rightPointerTransform, wristBones[1], fingerBones[1])) {
      Local<Object> pointerObj = Nan::New<Object>();
      pointerObj->Set(JS_STR("position"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightPointerTransform.position.values, 3 * sizeof(float)), 0, 3));
      pointerObj->Set(JS_STR("rotation"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightPointerTransform.rotation.values, 4 * sizeof(float)), 0, 4));
      obj->Set(JS_STR("pointer"), pointerObj);
    } else {
      obj->Set(JS_STR("pointer"), Nan::Null());
    }
    if (getHandGripTransform(rightGripTransform, wristBones[1], fingerBones[1])) {
      Local<Object> gripObj = Nan::New<Object>();
      gripObj->Set(JS_STR("position"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightGripTransform.position.values, 3 * sizeof(float)), 0, 3));
      gripObj->Set(JS_STR("rotation"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightGripTransform.rotation.values, 4 * sizeof(float)), 0, 4));
      obj->Set(JS_STR("grip"), gripObj);
    } else {
      obj->Set(JS_STR("grip"), Nan::Null());
    }
    
    Local<Array> wristArray = Nan::New<Array>(4);
    for (size_t i = 0; i < 4; i++) {
      Local<Value> boneVal;

      if (*(uint32_t *)&wristBones[1][i][0]) {
        boneVal = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)&wristBones[1][i][1], 3 * sizeof(float)), 0, 3);
      } else {
        boneVal = Nan::Null();
      }

      wristArray->Set(i, boneVal);
    }
    obj->Set(JS_STR("wrist"), wristArray);

    Local<Array> fingersArray = Nan::New<Array>(5);
    for (size_t i = 0; i < 5; i++) {
      Local<Array> bonesArray = Nan::New<Array>(4);

      for (size_t j = 0; j < 4; j++) {
        Local<Value> boneVal;

        if (*(uint32_t *)&fingerBones[1][i][j][0]) {
          boneVal = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)&fingerBones[1][i][j][1], 3 * sizeof(float)), 0, 3);
        } else {
          boneVal = Nan::Null();
        }

        bonesArray->Set(j, boneVal);
      }

      fingersArray->Set(i, bonesArray);
    }
    obj->Set(JS_STR("fingers"), fingersArray);

    const char *gesture = gestureCategoryToDescriptor(handData.right_hand_state.keypose);
    if (gesture) {
      obj->Set(JS_STR("gesture"), JS_STR(gesture));
    } else {
      obj->Set(JS_STR("gesture"), Nan::Null());
    }

    array->Set(JS_INT(numResults++), obj);
  }

  Local<Function> cb = Nan::New(this->cb);
  Local<Value> argv[] = {
    array,
  };
  asyncResource.MakeCallback(cb, sizeof(argv)/sizeof(argv[0]), argv);
}

NAN_METHOD(MLHandTracker::Destroy) {
  MLHandTracker *mlHandTracker = ObjectWrap::Unwrap<MLHandTracker>(info.This());

  handTrackers.erase(std::remove_if(handTrackers.begin(), handTrackers.end(), [&](MLHandTracker *h) -> bool {
    if (h == mlHandTracker) {
      delete h;
      return true;
    } else {
      return false;
    }
  }), handTrackers.end());
}

// MLEyeTracker

MLEyeTracker::MLEyeTracker() {}

MLEyeTracker::~MLEyeTracker() {}

NAN_METHOD(MLEyeTracker::New) {
  MLEyeTracker *mlEyeTracker = new MLEyeTracker();
  Local<Object> mlEyeTrackerObj = info.This();
  mlEyeTracker->Wrap(mlEyeTrackerObj);

  Nan::SetAccessor(mlEyeTrackerObj, JS_STR("position"), PositionGetter);
  Nan::SetAccessor(mlEyeTrackerObj, JS_STR("rotation"), RotationGetter);

  info.GetReturnValue().Set(mlEyeTrackerObj);

  eyeTrackers.push_back(mlEyeTracker);
}

Local<Function> MLEyeTracker::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("MLEyeTracker"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "destroy", Destroy);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_GETTER(MLEyeTracker::PositionGetter) {
  // Nan::HandleScope scope;
  MLEyeTracker *mlEyeTracker = ObjectWrap::Unwrap<MLEyeTracker>(info.This());

  Local<Float32Array> positionArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 3 * sizeof(float)), 0, 3);
  positionArray->Set(0, JS_NUM(mlEyeTracker->transform.position.x));
  positionArray->Set(1, JS_NUM(mlEyeTracker->transform.position.y));
  positionArray->Set(2, JS_NUM(mlEyeTracker->transform.position.z));

  info.GetReturnValue().Set(positionArray);
}

NAN_GETTER(MLEyeTracker::RotationGetter) {
  // Nan::HandleScope scope;
  MLEyeTracker *mlEyeTracker = ObjectWrap::Unwrap<MLEyeTracker>(info.This());

  Local<Float32Array> rotationArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 4 * sizeof(float)), 0, 4);
  rotationArray->Set(0, JS_NUM(mlEyeTracker->transform.rotation.x));
  rotationArray->Set(1, JS_NUM(mlEyeTracker->transform.rotation.y));
  rotationArray->Set(2, JS_NUM(mlEyeTracker->transform.rotation.z));
  rotationArray->Set(3, JS_NUM(mlEyeTracker->transform.rotation.w));

  info.GetReturnValue().Set(rotationArray);
}

void MLEyeTracker::Poll(MLSnapshot *snapshot) {
  const MLCoordinateFrameUID &id = eyeStaticData.fixation;
  if (MLSnapshotGetTransform(snapshot, &id, &this->transform) == MLResult_Ok) {
    // nothing
  } else {
    ML_LOG(Error, "%s: ML failed to get eye transform!", application_name);
  }
}

NAN_METHOD(MLEyeTracker::Destroy) {
  MLEyeTracker *mlEyeTracker = ObjectWrap::Unwrap<MLEyeTracker>(info.This());

  eyeTrackers.erase(std::remove_if(eyeTrackers.begin(), eyeTrackers.end(), [&](MLEyeTracker *e) -> bool {
    if (e == mlEyeTracker) {
      delete e;
      return true;
    } else {
      return false;
    }
  }));
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

MLContext::MLContext() : position{0, 0, 0}, rotation{0, 0, 0, 1} {}

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
  Nan::SetMethod(ctorFn, "RequestHandTracking", RequestHandTracking);
  Nan::SetMethod(ctorFn, "RequestMeshing", RequestMeshing);
  Nan::SetMethod(ctorFn, "RequestDepthPopulation", RequestDepthPopulation);
  Nan::SetMethod(ctorFn, "RequestPlaneTracking", RequestPlaneTracking);
  Nan::SetMethod(ctorFn, "RequestEyeTracking", RequestEyeTracking);
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

const char *meshVsh = "\
#version 330\n\
\n\
in vec3 position;\n\
\n\
uniform mat4 projectionMatrix;\n\
uniform mat4 modelViewMatrix;\n\
\n\
void main() {\n\
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);\n\
}\n\
";
const char *meshFsh = "\
#version 330\n\
\n\
out vec4 fragColor;\n\
\n\
void main() {\n\
  fragColor = vec4(1.0);\n\
}\n\
";
NAN_METHOD(MLContext::InitLifecycle) {
  if (info[0]->IsFunction() && info[1]->IsFunction()) {
    eventsCb.Reset(Local<Function>::Cast(info[0]));
    keyboardEventsCb.Reset(Local<Function>::Cast(info[1]));

    uv_async_init(uv_default_loop(), &eventsAsync, RunEventsInMainThread);
    uv_async_init(uv_default_loop(), &keyboardEventsAsync, RunKeyboardEventsInMainThread);

    mlMesherConstructor.Reset(MLMesher::Initialize(Isolate::GetCurrent()));
    mlPlaneTrackerConstructor.Reset(MLPlaneTracker::Initialize(Isolate::GetCurrent()));
    mlHandTrackerConstructor.Reset(MLHandTracker::Initialize(Isolate::GetCurrent()));
    mlEyeTrackerConstructor.Reset(MLEyeTracker::Initialize(Isolate::GetCurrent()));

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

  {
    glGenVertexArrays(1, &mlContext->meshVao);
    // glBindVertexArray(meshVao);

    // vertex Shader
    mlContext->meshVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(mlContext->meshVertex, 1, &meshVsh, NULL);
    glCompileShader(mlContext->meshVertex);
    GLint success;
    glGetShaderiv(mlContext->meshVertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(mlContext->meshVertex, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML vertex shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // fragment Shader
    mlContext->meshFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(mlContext->meshFragment, 1, &meshFsh, NULL);
    glCompileShader(mlContext->meshFragment);
    glGetShaderiv(mlContext->meshFragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(mlContext->meshFragment, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML fragment shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // shader Program
    mlContext->meshProgram = glCreateProgram();
    glAttachShader(mlContext->meshProgram, mlContext->meshVertex);
    glAttachShader(mlContext->meshProgram, mlContext->meshFragment);
    glLinkProgram(mlContext->meshProgram);
    glGetProgramiv(mlContext->meshProgram, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(mlContext->meshProgram, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML program linking failed\n" << infoLog << std::endl;
      return;
    }

    mlContext->positionLocation = glGetAttribLocation(mlContext->meshProgram, "position");
    if (mlContext->positionLocation == -1) {
      std::cout << "ML program failed to get attrib location for 'position'" << std::endl;
      return;
    }
    mlContext->modelViewMatrixLocation = glGetUniformLocation(mlContext->meshProgram, "modelViewMatrix");
    if (mlContext->modelViewMatrixLocation == -1) {
      std::cout << "ML program failed to get uniform location for 'modelViewMatrix'" << std::endl;
      return;
    }
    mlContext->projectionMatrixLocation = glGetUniformLocation(mlContext->meshProgram, "projectionMatrix");
    if (mlContext->projectionMatrixLocation == -1) {
      std::cout << "ML program failed to get uniform location for 'projectionMatrix'" << std::endl;
      return;
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(mlContext->meshVertex);
    glDeleteShader(mlContext->meshFragment);
  }

  MLGraphicsRenderTargetsInfo renderTargetsInfo;
  if (MLGraphicsGetRenderTargets(mlContext->graphics_client, &renderTargetsInfo) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to get graphics render targets.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  /* if (renderTargetsInfo.num_virtual_cameras != 2) {
    ML_LOG(Error, "Invalid graphics render targets num cameras: %u", renderTargetsInfo.num_virtual_cameras);
    info.GetReturnValue().Set(Nan::Null());
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
  handTrackingConfig.keypoints_filter_level = MLKeypointFilterLevel_1;
  handTrackingConfig.pose_filter_level = MLPoseFilterLevel_1;
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

inline MLMat4f composeMatrix(
  const MLVec3f &position = MLVec3f{0,0,0},
  const MLQuaternionf &quaternion = MLQuaternionf{0,0,0,1},
  const MLVec3f &scale = MLVec3f{1,1,1}
) {
  MLMat4f result;

  float	*te = result.matrix_colmajor;

  float x = quaternion.x, y = quaternion.y, z = quaternion.z, w = quaternion.w;
  float x2 = x + x,	y2 = y + y, z2 = z + z;
  float xx = x * x2, xy = x * y2, xz = x * z2;
  float yy = y * y2, yz = y * z2, zz = z * z2;
  float wx = w * x2, wy = w * y2, wz = w * z2;

  float sx = scale.x, sy = scale.y, sz = scale.z;

  te[ 0 ] = ( 1 - ( yy + zz ) ) * sx;
  te[ 1 ] = ( xy + wz ) * sx;
  te[ 2 ] = ( xz - wy ) * sx;
  te[ 3 ] = 0;

  te[ 4 ] = ( xy - wz ) * sy;
  te[ 5 ] = ( 1 - ( xx + zz ) ) * sy;
  te[ 6 ] = ( yz + wx ) * sy;
  te[ 7 ] = 0;

  te[ 8 ] = ( xz + wy ) * sz;
  te[ 9 ] = ( yz - wx ) * sz;
  te[ 10 ] = ( 1 - ( xx + yy ) ) * sz;
  te[ 11 ] = 0;

  te[ 12 ] = position.x;
  te[ 13 ] = position.y;
  te[ 14 ] = position.z;
  te[ 15 ] = 1;

  return result;
}

inline MLMat4f invertMatrix(const MLMat4f &matrix) {
  MLMat4f result;

  float	*te = result.matrix_colmajor;
  const float *me = matrix.matrix_colmajor;
  float n11 = me[ 0 ], n21 = me[ 1 ], n31 = me[ 2 ], n41 = me[ 3 ],
    n12 = me[ 4 ], n22 = me[ 5 ], n32 = me[ 6 ], n42 = me[ 7 ],
    n13 = me[ 8 ], n23 = me[ 9 ], n33 = me[ 10 ], n43 = me[ 11 ],
    n14 = me[ 12 ], n24 = me[ 13 ], n34 = me[ 14 ], n44 = me[ 15 ],

    t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44,
    t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44,
    t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44,
    t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

  float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;

  if ( det == 0 ) {

    std::cout << "ML can't invert matrix, determinant is 0" << std::endl;

    return MLMat4f{
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1,
    };

  }

  float detInv = 1 / det;

  te[ 0 ] = t11 * detInv;
  te[ 1 ] = ( n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44 ) * detInv;
  te[ 2 ] = ( n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44 ) * detInv;
  te[ 3 ] = ( n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43 ) * detInv;

  te[ 4 ] = t12 * detInv;
  te[ 5 ] = ( n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44 ) * detInv;
  te[ 6 ] = ( n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44 ) * detInv;
  te[ 7 ] = ( n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43 ) * detInv;

  te[ 8 ] = t13 * detInv;
  te[ 9 ] = ( n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44 ) * detInv;
  te[ 10 ] = ( n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44 ) * detInv;
  te[ 11 ] = ( n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43 ) * detInv;

  te[ 12 ] = t14 * detInv;
  te[ 13 ] = ( n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34 ) * detInv;
  te[ 14 ] = ( n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34 ) * detInv;
  te[ 15 ] = ( n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33 ) * detInv;

  return result;
}

NAN_METHOD(MLContext::WaitGetPoses) {
  if (info[0]->IsObject() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber() && info[4]->IsFloat32Array() && info[5]->IsFloat32Array() && info[6]->IsFloat32Array()) {
    if (application_context.dummy_value == DummyValue::RUNNING) {
      MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());
      WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));

      GLuint framebuffer = info[1]->Uint32Value();
      GLuint width = info[2]->Uint32Value();
      GLuint height = info[3]->Uint32Value();
      Local<Float32Array> transformArray = Local<Float32Array>::Cast(info[4]);
      Local<Float32Array> projectionArray = Local<Float32Array>::Cast(info[5]);
      Local<Float32Array> controllersArray = Local<Float32Array>::Cast(info[6]);

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

        if (depthEnabled) {
          glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

          glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
          glClearColor(0.0, 0.0, 0.0, 1.0);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
          glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

          glBindVertexArray(mlContext->meshVao);

          glUseProgram(mlContext->meshProgram);

          for (const auto &iter : meshBuffers) {
            const MeshBuffer &meshBuffer = iter.second;

            if (meshBuffer.numIndices > 0) {
              glBindBuffer(GL_ARRAY_BUFFER, meshBuffer.positionBuffer);
              glEnableVertexAttribArray(mlContext->positionLocation);
              glVertexAttribPointer(mlContext->positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

              glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshBuffer.indexBuffer);

              for (int side = 0; side < 2; side++) {
                const MLGraphicsVirtualCameraInfo &cameraInfo = mlContext->virtual_camera_array.virtual_cameras[side];
                const MLTransform &transform = cameraInfo.transform;
                const MLMat4f &modelView = invertMatrix(composeMatrix(transform.position, transform.rotation));
                glUniformMatrix4fv(mlContext->modelViewMatrixLocation, 1, false, modelView.matrix_colmajor);

                const MLMat4f &projection = cameraInfo.projection;
                glUniformMatrix4fv(mlContext->projectionMatrixLocation, 1, false, projection.matrix_colmajor);

                glViewport(side * width/2, 0, width/2, height);

                glDrawElements(GL_TRIANGLES, meshBuffer.numIndices, GL_UNSIGNED_SHORT, 0);
              }
            }
          }

          if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
          } else {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
          }
          if (gl->HasProgramBinding()) {
            glUseProgram(gl->GetProgramBinding());
          } else {
            glUseProgram(0);
          }
          if (gl->viewportState.valid) {
            glViewport(gl->viewportState.x, gl->viewportState.y, gl->viewportState.w, gl->viewportState.h);
          } else {
            glViewport(0, 0, 1280, 1024);
          }
          if (gl->HasVertexArrayBinding()) {
            glBindVertexArray(gl->GetVertexArrayBinding());
          } else {
            glBindVertexArray(gl->defaultVao);
          }
          if (gl->HasBufferBinding(GL_ARRAY_BUFFER)) {
            glBindBuffer(GL_ARRAY_BUFFER, gl->GetBufferBinding(GL_ARRAY_BUFFER));
          } else {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
          }
          if (gl->colorMaskState.valid) {
            glColorMask(gl->colorMaskState.r, gl->colorMaskState.g, gl->colorMaskState.b, gl->colorMaskState.a);
          } else {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
          }
        }

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

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
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
  }
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

NAN_METHOD(MLContext::RequestMeshing) {
  Local<Function> mlMesherCons = Nan::New(mlMesherConstructor);
  Local<Object> mlMesherObj = mlMesherCons->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
  info.GetReturnValue().Set(mlMesherObj);
}

NAN_METHOD(MLContext::RequestPlaneTracking) {
  Local<Function> mlPlaneTrackerCons = Nan::New(mlPlaneTrackerConstructor);
  Local<Object> mlPlaneTrackerObj = mlPlaneTrackerCons->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
  info.GetReturnValue().Set(mlPlaneTrackerObj);
}

NAN_METHOD(MLContext::RequestHandTracking) {
  Local<Function> mlHandTrackerCons = Nan::New(mlHandTrackerConstructor);
  Local<Object> mlHandTrackerObj = mlHandTrackerCons->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
  info.GetReturnValue().Set(mlHandTrackerObj);
}

NAN_METHOD(MLContext::RequestEyeTracking) {
  Local<Function> mlEyeTrackerCons = Nan::New(mlEyeTrackerConstructor);
  Local<Object> mlEyeTrackerObj = mlEyeTrackerCons->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
  info.GetReturnValue().Set(mlEyeTrackerObj);
}

NAN_METHOD(MLContext::RequestDepthPopulation) {
  if (info[0]->IsBoolean()) {
    depthEnabled = info[0]->BooleanValue();
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

    std::unique_lock<std::mutex> lock(cameraRequestsMutex);
    cameraRequests.erase(std::remove_if(cameraRequests.begin(), cameraRequests.end(), [&](CameraRequest *c) -> bool {
      Local<Function> localCbFn = Nan::New(c->cbFn);
      if (localCbFn->StrictEquals(cbFn)) {
        delete c;
        return true;
      } else {
        return false;
      }
    }), cameraRequests.end());
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

void setFingerValue(const MLWristState &wristState, MLSnapshot *snapshot, float data[4][1 + 3]);
void setFingerValue(const MLThumbState &thumbState, MLSnapshot *snapshot, float data[4][1 + 3]);
void setFingerValue(const MLFingerState &fingerState, MLSnapshot *snapshot, float data[4][1 + 3]);
void setFingerValue(const MLKeyPointState &keyPointState, MLSnapshot *snapshot, float data[1 + 3]);
void setFingerValue(float data[1 + 3]);

void setFingerValue(const MLWristState &wristState, MLSnapshot *snapshot, float data[4][1 + 3]) {
  setFingerValue(wristState.radial, snapshot, data[0]);
  setFingerValue(wristState.ulnar, snapshot, data[1]);
  setFingerValue(wristState.center, snapshot, data[2]);
  setFingerValue(data[3]);
}
void setFingerValue(const MLThumbState &thumbState, MLSnapshot *snapshot, float data[4][1 + 3]) {
  setFingerValue(thumbState.cmc, snapshot, data[0]);
  setFingerValue(thumbState.mcp, snapshot, data[1]);
  setFingerValue(thumbState.ip, snapshot, data[2]);
  setFingerValue(thumbState.tip, snapshot, data[3]);
}
void setFingerValue(const MLFingerState &fingerState, MLSnapshot *snapshot, float data[4][1 + 3]) {
  setFingerValue(fingerState.mcp, snapshot, data[0]);
  setFingerValue(fingerState.pip, snapshot, data[1]);
  setFingerValue(fingerState.dip, snapshot, data[2]);
  setFingerValue(fingerState.tip, snapshot, data[3]);
}
void setFingerValue(const MLKeyPointState &keyPointState, MLSnapshot *snapshot, float data[1 + 3]) {
  uint32_t *uint32Data = (uint32_t *)data;
  float *floatData = (float *)data;

  if (keyPointState.is_valid) {
    MLTransform transform;
    MLResult result = MLSnapshotGetTransform(snapshot, &keyPointState.frame_id, &transform);
    if (result == MLResult_Ok) {
      // ML_LOG(Info, "%s: ML keypoint ok", application_name);

      uint32Data[0] = true;
      floatData[1] = transform.position.x;
      floatData[2] = transform.position.y;
      floatData[3] = transform.position.z;
    } else {
      // ML_LOG(Error, "%s: ML failed to get finger transform: %s", application_name, MLSnapshotGetResultString(result));

      uint32Data[0] = false;
      const MLVec3f &position = MLVec3f{0, 0, 0};
      floatData[1] = position.x;
      floatData[2] = position.y;
      floatData[3] = position.z;
    }
  } else {
    uint32Data[0] = false;
    const MLVec3f &position = MLVec3f{0, 0, 0};
    floatData[1] = position.x;
    floatData[2] = position.y;
    floatData[3] = position.z;
  }
}
void setFingerValue(float data[1 + 3]) {
  uint32_t *uint32Data = (uint32_t *)data;
  float *floatData = (float *)data;

  uint32Data[0] = false;
  const MLVec3f &position = MLVec3f{0, 0, 0};
  floatData[1] = position.x;
  floatData[2] = position.y;
  floatData[3] = position.z;
}

NAN_METHOD(MLContext::PrePollEvents) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(Local<Object>::Cast(info[0]));
  MLSnapshot *snapshot = nullptr;

  if (handTrackers.size() > 0) {
    MLResult result = MLHandTrackingGetData(handTracker, &handData);
    if (result == MLResult_Ok) {
      MLResult result = MLHandTrackingGetStaticData(handTracker, &handStaticData);
      if (result == MLResult_Ok) {
        if (!snapshot) {
          if (MLPerceptionGetSnapshot(&snapshot) != MLResult_Ok) {
            ML_LOG(Error, "%s: ML failed to get eye snapshot!", application_name);
          }
        }

        // setFingerValue(handData.left_hand_state, handBones[0][0]);
        setFingerValue(handStaticData.left.wrist, snapshot, wristBones[0]);
        setFingerValue(handStaticData.left.thumb, snapshot, fingerBones[0][0]);
        setFingerValue(handStaticData.left.index, snapshot, fingerBones[0][1]);
        setFingerValue(handStaticData.left.middle, snapshot, fingerBones[0][2]);
        setFingerValue(handStaticData.left.ring, snapshot, fingerBones[0][3]);
        setFingerValue(handStaticData.left.pinky, snapshot, fingerBones[0][4]);

        // setFingerValue(handData.left_hand_state, handBones[1][0]);
        setFingerValue(handStaticData.right.wrist, snapshot, wristBones[1]);
        setFingerValue(handStaticData.right.thumb, snapshot, fingerBones[1][0]);
        setFingerValue(handStaticData.right.index, snapshot, fingerBones[1][1]);
        setFingerValue(handStaticData.right.middle, snapshot, fingerBones[1][2]);
        setFingerValue(handStaticData.right.ring, snapshot, fingerBones[1][3]);
        setFingerValue(handStaticData.right.pinky, snapshot, fingerBones[1][4]);

        std::for_each(handTrackers.begin(), handTrackers.end(), [&](MLHandTracker *h) {
          h->Poll();
        });
      } else {
        ML_LOG(Error, "%s: Hand static data get failed! %x", application_name, result);
      }
    } else {
      ML_LOG(Error, "%s: Hand data get failed! %x", application_name, result);
    }
  }

  if (eyeTrackers.size() > 0) {
    if (MLEyeTrackingGetState(eyeTracker, &eyeState) != MLResult_Ok) {
      ML_LOG(Error, "%s: Eye get state failed!", application_name);
    }

    if (MLEyeTrackingGetStaticData(eyeTracker, &eyeStaticData) != MLResult_Ok) {
      ML_LOG(Error, "%s: Eye get static data failed!", application_name);
    }

    if (!snapshot) {
      if (MLPerceptionGetSnapshot(&snapshot) != MLResult_Ok) {
        ML_LOG(Error, "%s: ML failed to get eye snapshot!", application_name);
      }
    }
    std::for_each(eyeTrackers.begin(), eyeTrackers.end(), [&](MLEyeTracker *e) {
      e->Poll(snapshot);
    });
  }

  if ((meshers.size() > 0 || depthEnabled) && !meshInfoRequestPending && !meshRequestsPending) {
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

  if (planeTrackers.size() > 0 && !planesRequestPending) {
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

  {
    std::unique_lock<std::mutex> lock(cameraRequestsMutex);
    if (cameraRequests.size() > 0) {
      cameraRequestConditionVariable.notify_one();
    }
  }

  if (snapshot) {
    if (MLPerceptionReleaseSnapshot(snapshot) != MLResult_Ok) {
      ML_LOG(Error, "%s: ML failed to release eye snapshot!", application_name);
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
      meshRequestNewMap.clear();
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

          const std::string &id = id2String(meshBlockInfo.id);
          meshRequestNewMap[id] = (state == MLMeshingMeshState_New);
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
      MLMeshingBlockMesh *blockMeshes = mesh.data;
      uint32_t dataCount = mesh.data_count;

      for (uint32_t i = 0; i < dataCount; i++) {
        MLMeshingBlockMesh &blockMesh = blockMeshes[i];

        const MLMeshingResult &result = blockMesh.result;
        if (result == MLMeshingResult_Success || result == MLMeshingResult_PartialUpdate) {
          const std::string &id = id2String(blockMesh.id);

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
          meshBuffer->setBuffers((float *)(&blockMesh.vertex->values), blockMesh.vertex_count * 3, (float *)(&blockMesh.normal->values), blockMesh.index, blockMesh.index_count, meshRequestNewMap[id]);
        } else if (result == MLMeshingResult_Pending) {
          // nothing
        } else {
          ML_LOG(Error, "%s: ML mesh request poll failed: %x %x", application_name, i, result);
        }
      }

      std::for_each(meshers.begin(), meshers.end(), [&](MLMesher *m) {
        m->Poll();
      });

      if (meshRemovedList.size() > 0) {
        for (const MLCoordinateFrameUID &cfid : meshRemovedList) {
          const std::string &id = id2String(cfid);

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
          } else {
            // ML_LOG(Error, "%s: ML mesh clear failed to find mesh: %x %s", application_name, idbuf);
          }
        }
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
      std::for_each(planeTrackers.begin(), planeTrackers.end(), [&](MLPlaneTracker *p) {
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

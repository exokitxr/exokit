#if defined(LUMIN)

#include <magicleap.h>
#include <ml-math.h>
#include <uv.h>
#include <iostream>

using namespace v8;
using namespace std;

namespace ml {

const char application_name[] = "com.exokit.app";
constexpr int CAMERA_SIZE[] = {960, 540};

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

bool cameraConnected = false;
std::thread cameraThread;
// std::mutex cameraRequestsMutex;
// std::mutex cameraResponseMutex;
std::vector<CameraRequest *> cameraRequests;
uv_async_t cameraAsync;
bool cameraConvertPending = false;
uint8_t cameraRequestRgb[CAMERA_SIZE[0] * CAMERA_SIZE[1] * 3];
uint8_t *cameraRequestJpeg;
size_t cameraRequestSize;
uv_sem_t cameraConvertSem;
uv_async_t cameraConvertAsync;

MLHandle handTracker;
MLHandTrackingData handData;
MLHandTrackingKeyPose lastKeyposeLeft = MLHandTrackingKeyPose_NoHand;
MLHandTrackingKeyPose lastKeyposeRight = MLHandTrackingKeyPose_NoHand;
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
std::map<std::string, bool> meshRequestRemovedMap;
std::map<std::string, bool> meshRequestUnchangedMap;
std::map<std::string, MLVec3f> meshRequestCentersMap;
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
bool cameraMeshEnabled = false;

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
      return "pinch";
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
inline Local<Value> gestureCategoryToJsValue(MLHandTrackingKeyPose keyPose) {
  const char *gesture = gestureCategoryToDescriptor(keyPose);
  if (gesture) {
    return JS_STR(gesture);
  } else {
    return Nan::Null();
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

MeshBuffer::MeshBuffer(GLuint positionBuffer, GLuint normalBuffer, GLuint uvBuffer, GLuint indexBuffer) :
  positionBuffer(positionBuffer),
  normalBuffer(normalBuffer),
  uvBuffer(uvBuffer),
  indexBuffer(indexBuffer),
  texture(0),
  texture2(0),
  positions(nullptr),
  numPositions(0),
  normals(nullptr),
  indices(nullptr),
  numIndices(0),
  uvs(nullptr),
  numUvs(0),
  isNew(true),
  isUnchanged(false)
  {}
MeshBuffer::MeshBuffer(const MeshBuffer &meshBuffer) {
  positionBuffer = meshBuffer.positionBuffer;
  normalBuffer = meshBuffer.normalBuffer;
  uvBuffer = meshBuffer.uvBuffer;
  indexBuffer = meshBuffer.indexBuffer;
  texture = meshBuffer.texture;
  texture2 = meshBuffer.texture2;
  positions = meshBuffer.positions;
  numPositions = meshBuffer.numPositions;
  normals = meshBuffer.normals;
  indices = meshBuffer.indices;
  numIndices = meshBuffer.numIndices;
  uvs = meshBuffer.uvs;
  numUvs = meshBuffer.numUvs;
  isNew = meshBuffer.isNew;
  isUnchanged = meshBuffer.isUnchanged;
}
MeshBuffer::MeshBuffer() : positionBuffer(0), normalBuffer(0), uvBuffer(0), indexBuffer(0), texture(0), texture2(0), positions(nullptr), numPositions(0), normals(nullptr), indices(nullptr), numIndices(0), uvs(nullptr), numUvs(0), isNew(true), isUnchanged(false) {}

void MeshBuffer::setBuffers(float *positions, uint32_t numPositions, float *normals, uint16_t *indices, uint16_t numIndices, const std::vector<Uv> *uvs, bool isNew, bool isUnchanged, const MLVec3f &center) {
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(positions[0]), positions, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
  glBufferData(GL_ARRAY_BUFFER, uvs->size() * 2 * sizeof(float), uvs->data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
  glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(normals[0]), normals, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
  glBufferData(GL_ARRAY_BUFFER, numIndices * sizeof(indices[0]), indices, GL_DYNAMIC_DRAW);

  this->positions = positions;
  this->numPositions = numPositions;
  this->normals = normals;
  this->indices = indices;
  this->numIndices = numIndices;
  this->uvs = (float *)uvs->data();
  this->numUvs = uvs->size() * 2;
  this->isNew = isNew;
  this->isUnchanged = isUnchanged;
  this->center = center;
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
      const std::string &id = id2String(blockMesh.id);

      if (!meshRequestRemovedMap[id]) {
        auto iter = meshBuffers.find(id);

        if (iter != meshBuffers.end()) {
          const MeshBuffer &meshBuffer = iter->second;

          Local<Object> obj = Nan::New<Object>();
          obj->Set(JS_STR("id"), JS_STR(id));
          const char *type;
          if (meshBuffer.isNew) {
            type = "new";
          } else if (meshBuffer.isUnchanged) {
            type = "unchanged";
          } else {
            type = "update";
          }
          obj->Set(JS_STR("type"), JS_STR(type));

          Local<Object> positionBuffer = Nan::New<Object>();
          positionBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.positionBuffer));
          obj->Set(JS_STR("positionBuffer"), positionBuffer);
          Local<Float32Array> positionArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), meshBuffer.positions, meshBuffer.numPositions * sizeof(float)), 0, meshBuffer.numPositions);
          obj->Set(JS_STR("positionArray"), positionArray);
          obj->Set(JS_STR("positionCount"), JS_INT(meshBuffer.numPositions));

          Local<Object> normalBuffer = Nan::New<Object>();
          normalBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.normalBuffer));
          obj->Set(JS_STR("normalBuffer"), normalBuffer);
          Local<Float32Array> normalArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), meshBuffer.normals, meshBuffer.numPositions * sizeof(float)), 0, meshBuffer.numPositions);
          obj->Set(JS_STR("normalArray"), normalArray);
          obj->Set(JS_STR("normalCount"), JS_INT(meshBuffer.numPositions));

          Local<Object> uvBuffer = Nan::New<Object>();
          uvBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.uvBuffer));
          obj->Set(JS_STR("uvBuffer"), uvBuffer);
          Local<Float32Array> uvArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), meshBuffer.uvs, meshBuffer.numUvs * sizeof(float)), 0, meshBuffer.numUvs);
          obj->Set(JS_STR("uvArray"), uvArray);
          obj->Set(JS_STR("uvCount"), JS_INT(meshBuffer.numUvs));

          Local<Object> indexBuffer = Nan::New<Object>();
          indexBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.indexBuffer));
          obj->Set(JS_STR("indexBuffer"), indexBuffer);
          Local<Uint16Array> indexArray = Uint16Array::New(ArrayBuffer::New(Isolate::GetCurrent(), meshBuffer.indices, meshBuffer.numIndices * sizeof(uint16_t)), 0, meshBuffer.numIndices);
          obj->Set(JS_STR("indexArray"), indexArray);
          obj->Set(JS_STR("count"), JS_INT(meshBuffer.numIndices));

          Local<Value> textureVal;
          if (meshBuffer.texture) {
            Local<Object> textureObj = Nan::New<Object>();
            textureObj->Set(JS_STR("id"), JS_INT(meshBuffer.texture));
            textureVal = textureObj;
          } else {
            textureVal = Nan::Null();
          }
          obj->Set(JS_STR("texture"), textureVal);

          Local<Value> texture2Val;
          if (meshBuffer.texture2) {
            Local<Object> texture2Obj = Nan::New<Object>();
            texture2Obj->Set(JS_STR("id"), JS_INT(meshBuffer.texture2));
            texture2Val = texture2Obj;
          } else {
            texture2Val = Nan::Null();
          }
          obj->Set(JS_STR("texture2"), texture2Val);
          /* Local<Object> textureObj2 = Nan::New<Object>();
          textureObj2->Set(JS_STR("id"), JS_INT(application_context.mlContext->cameraMeshTexture));
          obj->Set(JS_STR("texture2"), textureObj2); */
          
          Local<Float32Array> centerArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.center.values, 3 * sizeof(float)), 0, 3);
          obj->Set(JS_STR("center"), centerArray);

          array->Set(numResults++, obj);
        } else {
          ML_LOG(Error, "%s: ML mesh poll failed to find mesh: %s", application_name, id.c_str());
        }
      } else {
        Local<Object> obj = Nan::New<Object>();
        obj->Set(JS_STR("id"), JS_STR(id));
        obj->Set(JS_STR("type"), JS_STR("remove"));

        array->Set(numResults++, obj);
      }
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
  Nan::SetAccessor(mlHandTrackerObj, JS_STR("ongesture"), OnGestureGetter, OnGestureSetter);

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

NAN_GETTER(MLHandTracker::OnGestureGetter) {
  // Nan::HandleScope scope;
  MLHandTracker *mlHandTracker = ObjectWrap::Unwrap<MLHandTracker>(info.This());

  Local<Function> ongesture = Nan::New(mlHandTracker->ongesture);
  info.GetReturnValue().Set(ongesture);
}

NAN_SETTER(MLHandTracker::OnGestureSetter) {
  // Nan::HandleScope scope;
  MLHandTracker *mlHandTracker = ObjectWrap::Unwrap<MLHandTracker>(info.This());

  if (value->IsFunction()) {
    Local<Function> localOngesture = Local<Function>::Cast(value);
    mlHandTracker->ongesture.Reset(localOngesture);
  } else {
    Nan::ThrowError("MLHandTracker::OnGestureSetter: invalid arguments");
  }
}

void MLHandTracker::Poll() {
  if (!this->cb.IsEmpty()) {
    Local<Object> asyncObject = Nan::New<Object>();
    AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "MLHandTracker::Poll");

    Local<Array> array = Nan::New<Array>();
    uint32_t numResults = 0;

    MLVec3f leftHandCenter;
    MLVec3f leftHandNormal;
    bool leftHandTransformValid;
    MLTransform leftPointerTransform;
    bool leftPointerTransformValid;
    MLTransform leftGripTransform;
    bool leftGripTransformValid;

    MLVec3f rightHandCenter;
    MLVec3f rightHandNormal;
    bool rightHandTransformValid;
    MLTransform rightPointerTransform;
    bool rightPointerTransformValid;
    MLTransform rightGripTransform;
    bool rightGripTransformValid;

    if (getHandBone(leftHandCenter, 0, wristBones, fingerBones)) {
      Local<Object> obj = Nan::New<Object>();

      obj->Set(JS_STR("hand"), JS_STR("left"));

      leftHandTransformValid = getHandTransform(leftHandCenter, leftHandNormal, wristBones[0], fingerBones[0], true);
      if (leftHandTransformValid) {
        obj->Set(JS_STR("center"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftHandCenter.values, 3 * sizeof(float)), 0, 3));
        obj->Set(JS_STR("normal"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftHandNormal.values, 3 * sizeof(float)), 0, 3));
      } else {
        leftHandNormal = {0, 1, 0};
      }

      leftPointerTransformValid = getHandPointerTransform(leftPointerTransform, wristBones[0], fingerBones[0], leftHandNormal);
      if (leftPointerTransformValid) {
        Local<Object> pointerObj = Nan::New<Object>();
        pointerObj->Set(JS_STR("position"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftPointerTransform.position.values, 3 * sizeof(float)), 0, 3));
        pointerObj->Set(JS_STR("rotation"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftPointerTransform.rotation.values, 4 * sizeof(float)), 0, 4));
        obj->Set(JS_STR("pointer"), pointerObj);
      } else {
        obj->Set(JS_STR("pointer"), Nan::Null());
      }
      leftGripTransformValid = getHandGripTransform(leftGripTransform, wristBones[0], fingerBones[0], leftHandNormal);
      if (leftGripTransformValid) {
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

      Local<Value> gestureVal = gestureCategoryToJsValue(handData.left_hand_state.keypose);
      obj->Set(JS_STR("gesture"), gestureVal);

      if (handData.left_hand_state.keypose != lastKeyposeLeft && !this->ongesture.IsEmpty()) {
        Local<Object> gestureObj = Nan::New<Object>();

        gestureObj->Set(JS_STR("hand"), JS_STR("left"));

        Local<Value> gesturePositionObj;
        Local<Value> gestureRotationObj;
        if (leftPointerTransformValid) {
          gesturePositionObj = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftPointerTransform.position.values, 3 * sizeof(float)), 0, 3);
          gestureRotationObj = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftPointerTransform.rotation.values, 4 * sizeof(float)), 0, 4);
        } else if (leftGripTransformValid) {
          gesturePositionObj = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftGripTransform.position.values, 3 * sizeof(float)), 0, 3);
          gestureRotationObj = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)leftGripTransform.rotation.values, 4 * sizeof(float)), 0, 4);
        } else {
          gesturePositionObj = Nan::Null();
          gestureRotationObj = Nan::Null();
        }
        gestureObj->Set(JS_STR("position"), gesturePositionObj);
        gestureObj->Set(JS_STR("rotation"), gestureRotationObj);

        gestureObj->Set(JS_STR("gesture"), gestureVal);
        gestureObj->Set(JS_STR("lastGesture"), gestureCategoryToJsValue(lastKeyposeLeft));

        Local<Function> ongesture = Nan::New(this->ongesture);
        Local<Value> argv[] = {
          gestureObj,
        };
        asyncResource.MakeCallback(ongesture, sizeof(argv)/sizeof(argv[0]), argv);
      }
      lastKeyposeLeft = handData.left_hand_state.keypose;

      array->Set(JS_INT(numResults++), obj);
    }

    if (getHandBone(rightHandCenter, 1, wristBones, fingerBones)) {
      Local<Object> obj = Nan::New<Object>();

      obj->Set(JS_STR("hand"), JS_STR("right"));

      rightHandTransformValid = getHandTransform(rightHandCenter, rightHandNormal, wristBones[1], fingerBones[1], false);
      if (rightHandTransformValid) {
        obj->Set(JS_STR("center"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightHandCenter.values, 3 * sizeof(float)), 0, 3));
        obj->Set(JS_STR("normal"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightHandNormal.values, 3 * sizeof(float)), 0, 3));
      } else {
        rightHandNormal = {0, 1, 0};
      }

      rightPointerTransformValid = getHandPointerTransform(rightPointerTransform, wristBones[1], fingerBones[1], rightHandNormal);
      if (rightPointerTransformValid) {
        Local<Object> pointerObj = Nan::New<Object>();
        pointerObj->Set(JS_STR("position"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightPointerTransform.position.values, 3 * sizeof(float)), 0, 3));
        pointerObj->Set(JS_STR("rotation"), Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightPointerTransform.rotation.values, 4 * sizeof(float)), 0, 4));
        obj->Set(JS_STR("pointer"), pointerObj);
      } else {
        obj->Set(JS_STR("pointer"), Nan::Null());
      }
      rightGripTransformValid = getHandGripTransform(rightGripTransform, wristBones[1], fingerBones[1], rightHandNormal);
      if (rightGripTransformValid) {
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

      Local<Value> gestureVal = gestureCategoryToJsValue(handData.right_hand_state.keypose);
      obj->Set(JS_STR("gesture"), gestureVal);

      if (handData.right_hand_state.keypose != lastKeyposeRight && !this->ongesture.IsEmpty()) {
        Local<Object> gestureObj = Nan::New<Object>();

        gestureObj->Set(JS_STR("hand"), JS_STR("right"));

        Local<Value> gesturePositionObj;
        Local<Value> gestureRotationObj;
        if (rightPointerTransformValid) {
          gesturePositionObj = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightPointerTransform.position.values, 3 * sizeof(float)), 0, 3);
          gestureRotationObj = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightPointerTransform.rotation.values, 4 * sizeof(float)), 0, 4);
        } else if (rightGripTransformValid) {
          gesturePositionObj = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightGripTransform.position.values, 3 * sizeof(float)), 0, 3);
          gestureRotationObj = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)rightGripTransform.rotation.values, 4 * sizeof(float)), 0, 4);
        } else {
          gesturePositionObj = Nan::Null();
          gestureRotationObj = Nan::Null();
        }
        gestureObj->Set(JS_STR("position"), gesturePositionObj);
        gestureObj->Set(JS_STR("rotation"), gestureRotationObj);

        gestureObj->Set(JS_STR("gesture"), gestureVal);
        gestureObj->Set(JS_STR("lastGesture"), gestureCategoryToJsValue(lastKeyposeRight));

        Local<Function> ongesture = Nan::New(this->ongesture);
        Local<Value> argv[] = {
          gestureObj,
        };
        asyncResource.MakeCallback(ongesture, sizeof(argv)/sizeof(argv[0]), argv);
      }
      lastKeyposeRight = handData.right_hand_state.keypose;

      array->Set(JS_INT(numResults++), obj);
    }

    Local<Function> cb = Nan::New(this->cb);
    Local<Value> argv[] = {
      array,
    };
    asyncResource.MakeCallback(cb, sizeof(argv)/sizeof(argv[0]), argv);
  }
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

  Nan::SetAccessor(mlEyeTrackerObj, JS_STR("fixation"), FixationGetter);
  Nan::SetAccessor(mlEyeTrackerObj, JS_STR("eyes"), EyesGetter);

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

NAN_GETTER(MLEyeTracker::FixationGetter) {
  // Nan::HandleScope scope;
  MLEyeTracker *mlEyeTracker = ObjectWrap::Unwrap<MLEyeTracker>(info.This());

  Local<Object> obj = Nan::New<Object>();

  Local<Float32Array> positionArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 3 * sizeof(float)), 0, 3);
  positionArray->Set(0, JS_NUM(mlEyeTracker->transform.position.x));
  positionArray->Set(1, JS_NUM(mlEyeTracker->transform.position.y));
  positionArray->Set(2, JS_NUM(mlEyeTracker->transform.position.z));
  obj->Set(JS_STR("position"), positionArray);

  Local<Float32Array> rotationArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 4 * sizeof(float)), 0, 4);
  rotationArray->Set(0, JS_NUM(mlEyeTracker->transform.rotation.x));
  rotationArray->Set(1, JS_NUM(mlEyeTracker->transform.rotation.y));
  rotationArray->Set(2, JS_NUM(mlEyeTracker->transform.rotation.z));
  rotationArray->Set(3, JS_NUM(mlEyeTracker->transform.rotation.w));
  obj->Set(JS_STR("rotation"), rotationArray);

  info.GetReturnValue().Set(obj);
}

NAN_GETTER(MLEyeTracker::EyesGetter) {
  // Nan::HandleScope scope;
  MLEyeTracker *mlEyeTracker = ObjectWrap::Unwrap<MLEyeTracker>(info.This());

  Local<Array> array = Nan::New<Array>(2);
  for (size_t i = 0; i < 2; i++) {
    Local<Object> obj = Nan::New<Object>();

    const MLTransform &eyeTransform = i == 0 ? mlEyeTracker->leftTransform : mlEyeTracker->rightTransform;
    Local<Float32Array> positionArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 3 * sizeof(float)), 0, 3);
    positionArray->Set(0, JS_NUM(eyeTransform.position.x));
    positionArray->Set(1, JS_NUM(eyeTransform.position.y));
    positionArray->Set(2, JS_NUM(eyeTransform.position.z));
    obj->Set(JS_STR("position"), positionArray);

    Local<Float32Array> rotationArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), 4 * sizeof(float)), 0, 4);
    rotationArray->Set(0, JS_NUM(eyeTransform.rotation.x));
    rotationArray->Set(1, JS_NUM(eyeTransform.rotation.y));
    rotationArray->Set(2, JS_NUM(eyeTransform.rotation.z));
    rotationArray->Set(3, JS_NUM(eyeTransform.rotation.w));
    obj->Set(JS_STR("rotation"), rotationArray);

    const bool &eyeBlink = i == 0 ? mlEyeTracker->leftBlink : mlEyeTracker->rightBlink;
    obj->Set(JS_STR("blink"), JS_BOOL(eyeBlink));

    array->Set(i, obj);
  }

  info.GetReturnValue().Set(array);
}

void MLEyeTracker::Poll(MLSnapshot *snapshot) {
  if (MLSnapshotGetTransform(snapshot, &eyeStaticData.fixation, &this->transform) == MLResult_Ok) {
    // nothing
  } else {
    ML_LOG(Error, "%s: ML failed to get eye fixation transform!", application_name);
  }

  if (MLSnapshotGetTransform(snapshot, &eyeStaticData.left_center, &this->leftTransform) == MLResult_Ok) {
    // nothing
  } else {
    ML_LOG(Error, "%s: ML failed to get left eye center transform!", application_name);
  }
  this->leftBlink = eyeState.left_blink;

  if (MLSnapshotGetTransform(snapshot, &eyeStaticData.right_center, &this->rightTransform) == MLResult_Ok) {
    // nothing
  } else {
    ML_LOG(Error, "%s: ML failed to get right eye center transform!", application_name);
  }
  this->rightBlink = eyeState.right_blink;
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
  uv_async_send(&cameraAsync);
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
  /* if (!cameraResponsePending) {
    {
      std::unique_lock<std::mutex> lock(cameraRequestsMutex);
      std::for_each(cameraRequests.begin(), cameraRequests.end(), [&](CameraRequest *c) {
        c->Set(output);
      });
    }
    cameraResponsePending = true;
  } */
}

// CameraRequest

CameraRequest::CameraRequest(Local<Function> cbFn) : cbFn(cbFn) {}

void CameraRequest::Set(int width, int height, uint8_t *data, size_t size) {
  this->width = width;
  this->height = height;
  // this->stride = stride;

  Local<ArrayBuffer> arrayBuffer;
  if (size > 0) {
    // std::cout << "got jpeg " << size << std::endl;

    arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), size);
    memcpy(arrayBuffer->GetContents().Data(), data, size);
  } else {
    // std::cout << "failed to get jpeg " << size << std::endl;

    arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), 4);
  }
  this->data.Reset(arrayBuffer);
}

void CameraRequest::Poll() {
  Local<Object> asyncObject = Nan::New<Object>();
  AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "cameraRequest");

  Local<Function> cbFn = Nan::New(this->cbFn);

  Local<Object> obj = Nan::New<Object>();
  obj->Set(JS_STR("data"), Nan::New(data));
  data.Reset();
  obj->Set(JS_STR("width"), JS_INT(width));
  obj->Set(JS_STR("height"), JS_INT(height));
  // obj->Set(JS_STR("bpp"), JS_INT(bpp));
  // obj->Set(JS_STR("stride"), JS_INT(stride));

  Local<Value> argv[] = {
    obj,
  };
  asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);
}

MLContext::MLContext() : window(nullptr), gl(nullptr), position{0, 0, 0}, rotation{0, 0, 0, 1}, cameraInTexture(0), contentTexture(0), cameraOutTexture(0), cameraFbo(0) {}

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
  Nan::SetMethod(ctorFn, "RequestCameraMesh", RequestCameraMesh);
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

void RunCameraInMainThread(uv_async_t *handle) {
  Nan::HandleScope scope;

  if (!cameraConvertPending) {
    MLHandle output;
    MLResult result = MLCameraGetPreviewStream(&output);
    if (result == MLResult_Ok) {
      ANativeWindowBuffer_t *aNativeWindowBuffer = (ANativeWindowBuffer_t *)output;

      MLContext *mlContext = application_context.mlContext;
      NATIVEwindow *window = application_context.window;
      WebGLRenderingContext *gl = application_context.gl;

      EGLImageKHR yuv_img = eglCreateImageKHR(window->display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, (EGLClientBuffer)(void*)output, nullptr);

      glBindVertexArray(mlContext->cameraVao);
      glBindFramebuffer(GL_FRAMEBUFFER, mlContext->cameraFbo);
      glUseProgram(mlContext->cameraProgram);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_EXTERNAL_OES, mlContext->cameraInTexture);
      glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, yuv_img);
      glUniform1i(mlContext->cameraInTextureLocation, 0);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, mlContext->contentTexture);
      glUniform1i(mlContext->contentTextureLocation, 1);

      glViewport(0, 0, CAMERA_SIZE[0], CAMERA_SIZE[1]);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      eglDestroyImageKHR(window->display, yuv_img);

      glReadPixels(0, 0, CAMERA_SIZE[0], CAMERA_SIZE[1], GL_RGB, GL_UNSIGNED_BYTE, cameraRequestRgb);

      cameraConvertPending = true;
      uv_sem_post(&cameraConvertSem);

      if (gl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
      } else {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->defaultFramebuffer);
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
      if (gl->HasTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D)) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D));
      } else {
        glBindTexture(GL_TEXTURE_2D, 0);
      }
      if (gl->HasTextureBinding(GL_TEXTURE0, GL_TEXTURE_EXTERNAL_OES)) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, gl->GetTextureBinding(GL_TEXTURE0, GL_TEXTURE_EXTERNAL_OES));
      } else {
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
      }
      glActiveTexture(gl->activeTexture);
    } else {
      ML_LOG(Error, "%s: failed to get camera preview stream %x", application_name, result);
    }
  }
}

void RunCameraConvertInMainThread(uv_async_t *handle) {
  Nan::HandleScope scope;

  std::for_each(cameraRequests.begin(), cameraRequests.end(), [&](CameraRequest *c) {
    c->Set(CAMERA_SIZE[0], CAMERA_SIZE[1], cameraRequestJpeg, cameraRequestSize);
    c->Poll();
  });

  if (cameraRequestSize > 0) {
    SjpegFreeBuffer(cameraRequestJpeg);
  }

  cameraConvertPending = false;
}

// mesh depth shader
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

// camera stream shader
const char *cameraVsh = "\
#version 330\n\
\n\
in vec2 point;\n\
in vec2 uv;\n\
out vec2 vUvCamera;\n\
out vec2 vUvContent;\n\
\n\
float cameraAspectRatio = 960.0/540.0;\n\
float renderAspectRatio = 1280.0/960.0;\n\
float xfactor = cameraAspectRatio/renderAspectRatio;\n\
float factor = 1.4;\n\
\n\
void main() {\n\
  vUvCamera = vec2(uv.x, 1.0 - uv.y);\n\
  vUvContent = vec2(\n\
    ((uv.x * 0.5) - 0.25) * xfactor*factor + 0.25,\n\
    (uv.y - 0.5) * factor + 0.5\n\
  );\n\
  gl_Position = vec4(point, 1.0, 1.0);\n\
}\n\
";
const char *cameraFsh = "\
#version 330\n\
#extension GL_OES_EGL_image_external : enable\n\
\n\
in vec2 vUvCamera;\n\
in vec2 vUvContent;\n\
out vec4 fragColor;\n\
\n\
uniform samplerExternalOES cameraInTexture;\n\
uniform sampler2D contentTexture;\n\
\n\
void main() {\n\
  vec4 cameraIn = texture2D(cameraInTexture, vUvCamera);\n\
\n\
  if (vUvContent.x >= 0.0 && vUvContent.x <= 0.5 && vUvContent.y >= 0.0 && vUvContent.y <= 1.0) {\n\
    vec4 content = texture2D(contentTexture, vUvContent);\n\
    fragColor = vec4((cameraIn.rgb * (1.0 - content.a)) + (content.rgb * content.a), 1.0);\n\
  } else {\n\
    fragColor = vec4(cameraIn.rgb, 1.0);\n\
  }\n\
}\n\
";

// camera mesh shader
const char *cameraMeshVsh1 = "\
#version 330\n\
\n\
uniform mat4 projectionMatrix;\n\
uniform mat4 modelViewMatrix;\n\
\n\
in vec3 position;\n\
\n\
void main() {\n\
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);\n\
}\n\
";
const char *cameraMeshFsh1 = "\
#version 330\n\
\n\
out vec4 fragColor;\n\
\n\
void main() {\n\
  fragColor = vec4(gl_FragCoord.z, 0.5, 0.5, 1.0);\n\
}\n\
";
const char *cameraMeshVsh2 = "\
#version 330\n\
\n\
uniform mat4 projectionMatrix;\n\
uniform mat4 modelViewMatrix;\n\
\n\
in vec3 position;\n\
in vec2 uv;\n\
out vec2 vScreen;\n\
out vec2 vUv2;\n\
\n\
void main() {\n\
  vec4 screenPosition = projectionMatrix * modelViewMatrix * vec4(position, 1.0);\n\
  vScreen = screenPosition.xy / 2.0 + 0.5;\n\
  gl_Position = vec4((uv - 0.5) * 2.0, screenPosition.z, 1.0);\n\
  // gl_Position = screenPosition;\n\
  vUv2 = uv;\n\
}\n\
";
const char *cameraMeshFsh2 = "\
#version 330\n\
#extension GL_OES_EGL_image_external : enable\n\
\n\
uniform sampler2D prevStageTexture;\n\
uniform samplerExternalOES cameraInTexture;\n\
\n\
in vec2 vScreen;\n\
in vec2 vUv2;\n\
out vec4 fragColor;\n\
\n\
void main() {\n\
  float depth = texture2D(prevStageTexture, vScreen).r;\n\
  float depthDiff = abs(depth - gl_FragCoord.z);\n\
  fragColor = texture2D(cameraInTexture, vec2(vScreen.x, 1.0-vScreen.y));\n\
  fragColor.rgb = vec3(1.0)*depthDiff + fragColor.rgb*(1.0-depthDiff);\n\
  /* if (depthDiff < 0.2) {\n\
    fragColor = texture2D(cameraInTexture, vec2(vScreen.x, 1.0-vScreen.y));\n\
    fragColor.rgb = vec3(1.0)*depthDiff + fragColor.rgb*(1.0-depthDiff);\n\
  } else {\n\
    fragColor = vec4(vUv2.x, 0.0, vUv2.y, 1.0);\n\
    // discard;\n\
  } */\n\
  // fragColor.r += texture2D(prevStageTexture, vUv2).r * 0.00001;\n\
  // fragColor = vec4(vUv.x, (texture2D(prevStageTexture, vUv2).r + texture2D(cameraInTexture, vUv2).r) * 0.00001, vUv.y, 1.0);\n\
}\n\
";
NAN_METHOD(MLContext::InitLifecycle) {
  if (info[0]->IsFunction() && info[1]->IsFunction()) {
    eventsCb.Reset(Local<Function>::Cast(info[0]));
    keyboardEventsCb.Reset(Local<Function>::Cast(info[1]));

    uv_async_init(uv_default_loop(), &eventsAsync, RunEventsInMainThread);
    uv_async_init(uv_default_loop(), &keyboardEventsAsync, RunKeyboardEventsInMainThread);
    uv_async_init(uv_default_loop(), &cameraAsync, RunCameraInMainThread);
    uv_async_init(uv_default_loop(), &cameraConvertAsync, RunCameraConvertInMainThread);
    uv_sem_init(&cameraConvertSem, 0);

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
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[1]));

  application_context.mlContext = mlContext;
  application_context.window = window;
  application_context.gl = gl;

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

  unsigned int halfWidth = renderTargetsInfo.buffers[0].color.width;
  unsigned int width = halfWidth * 2;
  unsigned int height = renderTargetsInfo.buffers[0].color.height;

  // std::cout << "render info " << halfWidth << " " << height << std::endl;

  GLuint fbo;
  GLuint colorTex;
  GLuint depthStencilTex;
  GLuint msFbo;
  GLuint msColorTex;
  GLuint msDepthStencilTex;
  {
    bool ok = windowsystem::CreateRenderTarget(gl, width, height, 0, 0, 0, 0, &fbo, &colorTex, &depthStencilTex, &msFbo, &msColorTex, &msDepthStencilTex);
    if (!ok) {
      ML_LOG(Error, "%s: Failed to create ML present render context.", application_name);
      info.GetReturnValue().Set(Nan::Null());
      return;
    }
  }

  {
    // mesh shader
    glGenVertexArrays(1, &mlContext->meshVao);

    // vertex Shader
    GLuint meshVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(meshVertex, 1, &meshVsh, NULL);
    glCompileShader(meshVertex);
    GLint success;
    glGetShaderiv(meshVertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(meshVertex, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML mesh vertex shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // fragment Shader
    GLuint meshFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(meshFragment, 1, &meshFsh, NULL);
    glCompileShader(meshFragment);
    glGetShaderiv(meshFragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(meshFragment, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML mesh fragment shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // shader Program
    mlContext->meshProgram = glCreateProgram();
    glAttachShader(mlContext->meshProgram, meshVertex);
    glAttachShader(mlContext->meshProgram, meshFragment);
    glLinkProgram(mlContext->meshProgram);
    glGetProgramiv(mlContext->meshProgram, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(mlContext->meshProgram, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML mesh program linking failed\n" << infoLog << std::endl;
      return;
    }

    mlContext->meshPositionLocation = glGetAttribLocation(mlContext->meshProgram, "position");
    if (mlContext->meshPositionLocation == -1) {
      std::cout << "ML mesh program failed to get attrib location for 'position'" << std::endl;
      return;
    }
    mlContext->meshModelViewMatrixLocation = glGetUniformLocation(mlContext->meshProgram, "modelViewMatrix");
    if (mlContext->meshModelViewMatrixLocation == -1) {
      std::cout << "ML meshprogram failed to get uniform location for 'modelViewMatrix'" << std::endl;
      return;
    }
    mlContext->meshProjectionMatrixLocation = glGetUniformLocation(mlContext->meshProgram, "projectionMatrix");
    if (mlContext->meshProjectionMatrixLocation == -1) {
      std::cout << "ML mesh program failed to get uniform location for 'projectionMatrix'" << std::endl;
      return;
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(meshVertex);
    glDeleteShader(meshFragment);
  }

  {
    // camera shader
    glGenVertexArrays(1, &mlContext->cameraVao);
    glBindVertexArray(mlContext->cameraVao);

    // vertex Shader
    GLuint cameraVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cameraVertex, 1, &cameraVsh, NULL);
    glCompileShader(cameraVertex);
    GLint success;
    glGetShaderiv(cameraVertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraVertex, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera vertex shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // fragment Shader
    GLuint cameraFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cameraFragment, 1, &cameraFsh, NULL);
    glCompileShader(cameraFragment);
    glGetShaderiv(cameraFragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraFragment, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera fragment shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // shader Program
    mlContext->cameraProgram = glCreateProgram();
    glAttachShader(mlContext->cameraProgram, cameraVertex);
    glAttachShader(mlContext->cameraProgram, cameraFragment);
    glLinkProgram(mlContext->cameraProgram);
    glGetProgramiv(mlContext->cameraProgram, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(mlContext->cameraProgram, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera program linking failed\n" << infoLog << std::endl;
      return;
    }

    mlContext->cameraPointLocation = glGetAttribLocation(mlContext->cameraProgram, "point");
    if (mlContext->cameraPointLocation == -1) {
      std::cout << "ML camera program failed to get attrib location for 'point'" << std::endl;
      return;
    }
    mlContext->cameraUvLocation = glGetAttribLocation(mlContext->cameraProgram, "uv");
    if (mlContext->cameraUvLocation == -1) {
      std::cout << "ML camera program failed to get attrib location for 'uv'" << std::endl;
      return;
    }
    mlContext->cameraInTextureLocation = glGetUniformLocation(mlContext->cameraProgram, "cameraInTexture");
    if (mlContext->cameraInTextureLocation == -1) {
      std::cout << "ML camera program failed to get uniform location for 'cameraInTexture'" << std::endl;
      return;
    }
    mlContext->contentTextureLocation = glGetUniformLocation(mlContext->cameraProgram, "contentTexture");
    if (mlContext->contentTextureLocation == -1) {
      std::cout << "ML camera program failed to get uniform location for 'contentTexture'" << std::endl;
      return;
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(cameraVertex);
    glDeleteShader(cameraFragment);

    glGenBuffers(1, &mlContext->pointBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mlContext->pointBuffer);
    static const GLfloat points[] = {
      -1.0f, -1.0f,
      1.0f, -1.0f,
      -1.0f, 1.0f,
      1.0f, 1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(mlContext->cameraPointLocation);
    glVertexAttribPointer(mlContext->cameraPointLocation, 2, GL_FLOAT, false, 0, 0);

    glGenBuffers(1, &mlContext->uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mlContext->uvBuffer);
    static const GLfloat uvs[] = {
      0.0f, 1.0f,
      1.0f, 1.0f,
      0.0f, 0.0f,
      1.0f, 0.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(mlContext->cameraUvLocation);
    glVertexAttribPointer(mlContext->cameraUvLocation, 2, GL_FLOAT, false, 0, 0);

    glGenTextures(1, &mlContext->cameraInTexture);
    mlContext->contentTexture = colorTex;
    // glBindTexture(GL_TEXTURE_2D, mlContext->cameraInTexture);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenTextures(1, &mlContext->cameraOutTexture);
    // glBindTexture(GL_TEXTURE_2D, mlContext->cameraOutTexture);
    glBindTexture(GL_TEXTURE_2D, mlContext->cameraOutTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, CAMERA_SIZE[0], CAMERA_SIZE[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenFramebuffers(1, &mlContext->cameraFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->cameraFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mlContext->cameraOutTexture, 0);

    {
      GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
      if (result != GL_FRAMEBUFFER_COMPLETE) {
        ML_LOG(Error, "%s: Failed to create camera framebuffer: %x", application_name, result);
      }
    }
  }

  /* glGenTextures(1, &mlContext->cameraMeshTexture);
  glBindTexture(GL_TEXTURE_2D, mlContext->cameraMeshTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, CAMERA_SIZE[0], CAMERA_SIZE[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); */

  glGenTextures(1, &mlContext->cameraMeshDepthTexture);
  glBindTexture(GL_TEXTURE_2D, mlContext->cameraMeshDepthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, CAMERA_SIZE[0], CAMERA_SIZE[1], 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glGenFramebuffers(1, &mlContext->cameraMeshFbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->cameraMeshFbo);
  // glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mlContext->cameraMeshTexture, 0);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mlContext->cameraMeshDepthTexture, 0);

  /* {
    GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE) {
      ML_LOG(Error, "%s: Failed to create camera mesh framebuffer: %x", application_name, result);
    }
  } */

  {
    // camera mesh shader 1
    glGenVertexArrays(1, &mlContext->cameraMeshVao1);
    glBindVertexArray(mlContext->cameraMeshVao1);

    // vertex Shader
    GLuint cameraMeshVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cameraMeshVertex, 1, &cameraMeshVsh1, NULL);
    glCompileShader(cameraMeshVertex);
    GLint success;
    glGetShaderiv(cameraMeshVertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraMeshVertex, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera mesh 1 vertex shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // fragment Shader
    GLuint cameraMeshFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cameraMeshFragment, 1, &cameraMeshFsh1, NULL);
    glCompileShader(cameraMeshFragment);
    glGetShaderiv(cameraMeshFragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraMeshFragment, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera mesh 1 fragment shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // shader Program
    mlContext->cameraMeshProgram1 = glCreateProgram();
    glAttachShader(mlContext->cameraMeshProgram1, cameraMeshVertex);
    glAttachShader(mlContext->cameraMeshProgram1, cameraMeshFragment);
    glLinkProgram(mlContext->cameraMeshProgram1);
    glGetProgramiv(mlContext->cameraMeshProgram1, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(mlContext->cameraMeshProgram1, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera mesh 1 program linking failed\n" << infoLog << std::endl;
      return;
    }

    mlContext->cameraMeshPositionLocation1 = glGetAttribLocation(mlContext->cameraMeshProgram1, "position");
    if (mlContext->cameraMeshPositionLocation1 == -1) {
      std::cout << "ML camera mesh 1 program failed to get attrib location for 'position'" << std::endl;
      return;
    }
    /* mlContext->cameraMeshIndexLocation1 = glGetAttribLocation(mlContext->cameraMeshProgram1, "index");
    if (mlContext->cameraMeshIndexLocation1 == -1) {
      std::cout << "ML camera mesh 1 program failed to get attrib location for 'index'" << std::endl;
      return;
    } */
    mlContext->cameraMeshModelViewMatrixLocation1 = glGetUniformLocation(mlContext->cameraMeshProgram1, "modelViewMatrix");
    if (mlContext->cameraMeshModelViewMatrixLocation1 == -1) {
      std::cout << "ML camera mesh 1 program failed to get uniform location for 'modelViewMatrix'" << std::endl;
      return;
    }
    mlContext->cameraMeshProjectionMatrixLocation1 = glGetUniformLocation(mlContext->cameraMeshProgram1, "projectionMatrix");
    if (mlContext->cameraMeshProjectionMatrixLocation1 == -1) {
      std::cout << "ML camera mesh 1 program failed to get uniform location for 'projectionMatrix'" << std::endl;
      return;
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(cameraMeshVertex);
    glDeleteShader(cameraMeshFragment);
  }

  {
    // camera mesh shader 2
    glGenTextures(1, &mlContext->cameraMeshDepthTexture2);
    glBindTexture(GL_TEXTURE_2D, mlContext->cameraMeshDepthTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, CAMERA_SIZE[0], CAMERA_SIZE[1], 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glGenFramebuffers(1, &mlContext->cameraMeshFbo2);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->cameraMeshFbo2);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mlContext->cameraMeshDepthTexture2, 0);

    glGenVertexArrays(1, &mlContext->cameraMeshVao2);
    glBindVertexArray(mlContext->cameraMeshVao2);

    // vertex Shader
    GLuint cameraMeshVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cameraMeshVertex, 1, &cameraMeshVsh2, NULL);
    glCompileShader(cameraMeshVertex);
    GLint success;
    glGetShaderiv(cameraMeshVertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraMeshVertex, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera mesh 2 vertex shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // fragment Shader
    GLuint cameraMeshFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cameraMeshFragment, 1, &cameraMeshFsh2, NULL);
    glCompileShader(cameraMeshFragment);
    glGetShaderiv(cameraMeshFragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraMeshFragment, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera mesh 2 fragment shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // shader Program
    mlContext->cameraMeshProgram2 = glCreateProgram();
    glAttachShader(mlContext->cameraMeshProgram2, cameraMeshVertex);
    glAttachShader(mlContext->cameraMeshProgram2, cameraMeshFragment);
    glLinkProgram(mlContext->cameraMeshProgram2);
    glGetProgramiv(mlContext->cameraMeshProgram2, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(mlContext->cameraMeshProgram2, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera mesh 2 program linking failed\n" << infoLog << std::endl;
      return;
    }

    mlContext->cameraMeshPositionLocation2 = glGetAttribLocation(mlContext->cameraMeshProgram2, "position");
    if (mlContext->cameraMeshPositionLocation2 == -1) {
      std::cout << "ML camera mesh 2 program failed to get attrib location for 'position'" << std::endl;
      return;
    }
    mlContext->cameraMeshUvLocation2 = glGetAttribLocation(mlContext->cameraMeshProgram2, "uv");
    if (mlContext->cameraMeshUvLocation2 == -1) {
      std::cout << "ML camera mesh 2 program failed to get attrib location for 'uv'" << std::endl;
      return;
    }
    mlContext->cameraMeshModelViewMatrixLocation2 = glGetUniformLocation(mlContext->cameraMeshProgram2, "modelViewMatrix");
    if (mlContext->cameraMeshModelViewMatrixLocation2 == -1) {
      std::cout << "ML camera mesh 2 program failed to get uniform location for 'modelViewMatrix'" << std::endl;
      return;
    }
    mlContext->cameraMeshProjectionMatrixLocation2 = glGetUniformLocation(mlContext->cameraMeshProgram2, "projectionMatrix");
    if (mlContext->cameraMeshProjectionMatrixLocation2 == -1) {
      std::cout << "ML camera mesh 2 program failed to get uniform location for 'projectionMatrix'" << std::endl;
      return;
    }
    mlContext->cameraMeshPrevStageTextureLocation2 = glGetUniformLocation(mlContext->cameraMeshProgram2, "prevStageTexture");
    if (mlContext->cameraMeshPrevStageTextureLocation2 == -1) {
      std::cout << "ML camera mesh 2 program failed to get uniform location for 'prevStageTexture'" << std::endl;
      return;
    }
    mlContext->cameraMeshCameraInTextureLocation2 = glGetUniformLocation(mlContext->cameraMeshProgram2, "cameraInTexture");
    if (mlContext->cameraMeshCameraInTextureLocation2 == -1) {
      std::cout << "ML camera mesh 2 program failed to get uniform location for 'cameraInTexture'" << std::endl;
      return;
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(cameraMeshVertex);
    glDeleteShader(cameraMeshFragment);
  }

  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
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
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D)) {
    glBindFramebuffer(GL_TEXTURE_2D, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D));
  } else {
    glBindFramebuffer(GL_TEXTURE_2D, 0);
  }

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
      Nan::ThrowError("MLContext::Present failed to set keyboard callbacks");
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
  handTrackingConfig.keypose_config[MLHandTrackingKeyPose_Ok] = false;
  handTrackingConfig.keypose_config[MLHandTrackingKeyPose_C] = false;
  handTrackingConfig.keypose_config[MLHandTrackingKeyPose_L] = false;
  if (MLHandTrackingSetConfiguration(handTracker, &handTrackingConfig) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to set hand tracker config.", application_name);
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  MLMeshingSettings meshingSettings;
  if (MLMeshingInitSettings(&meshingSettings) != MLResult_Ok) {
    ML_LOG(Error, "%s: failed to initialize meshing settings", application_name);
  }
  meshingSettings.flags |= MLMeshingFlags_ComputeNormals /* | MLMeshingFlags_RemoveMeshSkirt */;
  {
    MLResult result = MLMeshingCreateClient(&meshTracker, &meshingSettings);
    if (result != MLResult_Ok) {
      ML_LOG(Error, "%s: failed to create mesh handle %x", application_name, result);
      Nan::ThrowError("MLContext::Present failed to create mesh handle");
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

  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"), JS_INT(halfWidth));
  result->Set(JS_STR("height"), JS_INT(height));
  result->Set(JS_STR("fbo"), JS_INT(fbo));
  result->Set(JS_STR("colorTex"), JS_INT(colorTex));
  result->Set(JS_STR("depthStencilTex"), JS_INT(depthStencilTex));
  result->Set(JS_STR("msFbo"), JS_INT(msFbo));
  result->Set(JS_STR("msColorTex"), JS_INT(msColorTex));
  result->Set(JS_STR("msDepthStencilTex"), JS_INT(msDepthStencilTex));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(MLContext::Exit) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(info.This());

  if (cameraConnected) {
    cameraConnected = false;

    uv_sem_post(&cameraConvertSem);
    cameraThread.join();

    MLResult result = MLCameraDisconnect();
    if (result != MLResult_Ok) {
      ML_LOG(Error, "%s: Failed to disconnect camera client: %x", application_name, result);
      info.GetReturnValue().Set(Nan::Null());
      return;
    }
  }

  if (MLGraphicsDestroyClient(&mlContext->graphics_client) != MLResult_Ok) {
    ML_LOG(Error, "%s: Failed to create graphics client.", application_name);
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
            float bumper = controllerState.button_state[MLInputControllerButton_Bumper] ? 1.0f : 0.0f;
            float home = controllerState.button_state[MLInputControllerButton_HomeTap] ? 1.0f : 0.0f;
            MLVec3f &touchPosAndForce = controllerState.touch_pos_and_force[0];

            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 0, JS_NUM(position.x));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 1, JS_NUM(position.y));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 2, JS_NUM(position.z));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 3, JS_NUM(orientation.x));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 4, JS_NUM(orientation.y));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 5, JS_NUM(orientation.z));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 6, JS_NUM(orientation.w));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 7, JS_NUM(trigger));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 8, JS_NUM(bumper));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 9, JS_NUM(home));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 10, JS_NUM(touchPosAndForce.x));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 11, JS_NUM(touchPosAndForce.y));
            controllersArray->Set((i*CONTROLLER_ENTRY_SIZE) + 12, JS_NUM(touchPosAndForce.z));
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
              glEnableVertexAttribArray(mlContext->meshPositionLocation);
              glVertexAttribPointer(mlContext->meshPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

              glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshBuffer.indexBuffer);

              for (int side = 0; side < 2; side++) {
                const MLGraphicsVirtualCameraInfo &cameraInfo = mlContext->virtual_camera_array.virtual_cameras[side];
                const MLTransform &transform = cameraInfo.transform;
                const MLMat4f &modelView = invertMatrix(composeMatrix(transform.position, transform.rotation));
                glUniformMatrix4fv(mlContext->meshModelViewMatrixLocation, 1, false, modelView.matrix_colmajor);

                const MLMat4f &projection = cameraInfo.projection;
                glUniformMatrix4fv(mlContext->meshProjectionMatrixLocation, 1, false, projection.matrix_colmajor);

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

MLResult connectCamera() {
  MLResult result = MLCameraConnect();
  if (result == MLResult_Ok) {
    cameraConnected = true;

    cameraThread = std::thread([]() -> void {
      for (;;) {
        uv_sem_wait(&cameraConvertSem);

        if (cameraConnected) {
          cameraRequestSize = SjpegCompress(cameraRequestRgb, CAMERA_SIZE[0], CAMERA_SIZE[1], 50.0f, &cameraRequestJpeg);

          uv_async_send(&cameraConvertAsync);
        } else {
          break;
        }
      }
    });
  }
  return result;
}

NAN_METHOD(MLContext::RequestCamera) {
  if (info[0]->IsFunction()) {
    if (!cameraConnected) {
      MLResult result = connectCamera();
      if (result != MLResult_Ok) {
        ML_LOG(Error, "%s: Failed to connect camera: %x", application_name, result);
        Nan::ThrowError("failed to connect camera");
        return;
      }
    }

    Local<Function> cbFn = Local<Function>::Cast(info[0]);
    {
      // std::unique_lock<std::mutex> lock(cameraRequestsMutex);

      CameraRequest *cameraRequest = new CameraRequest(cbFn);
      cameraRequests.push_back(cameraRequest);
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
      // std::unique_lock<std::mutex> lock(cameraRequestsMutex);

      cameraRequests.erase(std::remove_if(cameraRequests.begin(), cameraRequests.end(), [&](CameraRequest *c) -> bool {
        Local<Function> localCbFn = Nan::New(c->cbFn);
        if (localCbFn->StrictEquals(cbFn)) {
          delete c;
          return true;
        } else {
          return false;
        }
      }), cameraRequests.end());
    }
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(MLContext::RequestCameraMesh) {
  if (!cameraConnected) {
    MLResult result = connectCamera();
    if (result != MLResult_Ok) {
      ML_LOG(Error, "%s: Failed to connect camera: %x", application_name, result);
      Nan::ThrowError("failed to connect camera");
      return;
    }
  }

  cameraMeshEnabled = true;
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

  if (snapshot) {
    if (MLPerceptionReleaseSnapshot(snapshot) != MLResult_Ok) {
      ML_LOG(Error, "%s: ML failed to release eye snapshot!", application_name);
    }
  }
}

const MLVec3f &I = {1,0,0};
const MLVec3f &J = {0,1,0};
const MLVec3f &K = {0,0,1};
float approxAtan2(float y, float x) {
  int o = 0;
  if (y < 0) { x = -x; y = -y; o |= 4; }
  if (x <= 0) { float t = x; x = y; y = -t; o |= 2; }
  if (x <= y) { float t = y - x; x += y; y = t; o |= 1; }
  return o + y / x;
}
float getNormalSortKey(const MLVec3f &r, const MLVec3f &c, const MLVec3f &n, const MLVec3f &pp, const MLVec3f &qp) {
  const MLVec3f &rmc = subVectors(r, c);
  return approxAtan2(dotVectors(n, crossVectors(rmc, pp)), dotVectors(n, crossVectors(rmc, qp)));
}
const MLVec3f &longestVector(const MLVec3f &a, const MLVec3f &b, const MLVec3f &c) {
  const float aLength = vectorLengthSq(a);
  const float bLength = vectorLengthSq(b);
  const float cLength = vectorLengthSq(c);
  if (aLength >= bLength && aLength >= cLength) {
    return a;
  } else if (bLength >= aLength && bLength >= cLength) {
    return b;
  } else {
    return c;
  }
}
class LayerQueueEntry {
public:
  uint16_t vertexIndex;
  unsigned int depth;
  size_t radialIndex;
  float radiusStart;
  float radiusEnd;
};
constexpr float SQRT2 = 1.41421356237;
void sortConnectedVertices(MLVec3f *vertex, uint16_t vertexIndex, const MLVec3f &n, std::vector<uint16_t> &connectedVertices) {
  // std::cout << "sort vertices 1" << std::endl;

  const MLVec3f &c = vertex[vertexIndex];
  const MLVec3f &ni = crossVectors(n, I);
  const MLVec3f &nj = crossVectors(n, J);
  const MLVec3f &nk = crossVectors(n, K);
  const MLVec3f &pp = longestVector(ni, nj, nk);
  const MLVec3f &qp = crossVectors(n, pp);
  std::vector<std::pair<uint16_t, float>> connectedVerticesSorts(connectedVertices.size());
  for (size_t j = 0; j < connectedVertices.size(); j++) {
    const uint16_t &vertexIndex = connectedVertices[j];
    const MLVec3f &v = vertex[vertexIndex];
    const float sortKey = getNormalSortKey(v, c, n, pp, qp);
    connectedVerticesSorts[j] = std::pair<uint16_t, float>(vertexIndex, sortKey);
  }
  std::sort(connectedVerticesSorts.begin(), connectedVerticesSorts.end(), [](const std::pair<uint16_t, float> &a, const std::pair<uint16_t, float> &b) -> bool {
    return a.second < b.second;
  });
  for (size_t j = 0; j < connectedVertices.size(); j++) {
    connectedVertices[j] = connectedVerticesSorts[j].first;
  }

  // std::cout << "sort vertices 2" << std::endl;
}
void getUvs(MLVec3f *vertex, uint32_t vertex_count, uint16_t *index, uint16_t index_count, std::vector<Uv> *uvs) {
  if (index_count > 0) {
    // std::cout << "get uvs 1" << std::endl;

    std::vector<std::vector<uint16_t>> triangles(vertex_count); // vertex -> list of indices
    std::vector<MLVec3f> faceNormals(index_count); // index -> face normal
    for (uint16_t i = 0; i < index_count / 3; i++) {
      const uint16_t &a = index[i * 3];
      triangles[a].push_back(i);
      const uint16_t &b = index[i * 3 + 1];
      triangles[b].push_back(i);
      const uint16_t &c = index[i * 3 + 2];
      triangles[c].push_back(i);

      faceNormals[i] = getTriangleNormal(vertex[a], vertex[b], vertex[c]);
    }

    // std::cout << "get uvs 2" << std::endl;

    std::vector<std::vector<uint16_t>> adjacentVertices(vertex_count); // vertex -> list of vertices
    std::vector<std::vector<uint16_t>> islands; // list of lists of connected vertices
    {
      std::vector<bool> vertexSeenIndex(vertex_count, false);
      for (uint16_t i = 0; i < (uint16_t)vertex_count; i++) {
        if (!vertexSeenIndex[i]) {
          std::vector<uint16_t> island;
          island.reserve(128);
          std::deque<uint16_t> vertexQueue = {i};
          vertexSeenIndex[i] = true;

          while (vertexQueue.size() > 0) {
            const uint16_t &vertexIndex = vertexQueue.front();

            // add current vertex to island
            island.push_back(vertexIndex);

            // get connected indices
            std::vector<uint16_t> &connectedVertices = adjacentVertices[vertexIndex];
            MLVec3f vertexNormal = {0,0,0};
            const std::vector<uint16_t> &connectedIndices = triangles[vertexIndex];
            for (size_t j = 0; j < connectedIndices.size(); j++) {
              const uint16_t &connectedIndex = connectedIndices[j];

              const uint16_t &a = index[connectedIndex * 3];
              if (a != vertexIndex) {
                connectedVertices.push_back(a);
              }
              if (!vertexSeenIndex[a]) {
                vertexQueue.push_back(a);
                vertexSeenIndex[a] = true;
              }
              const uint16_t &b = index[connectedIndex * 3 + 1];
              if (b != vertexIndex) {
                connectedVertices.push_back(b);
              }
              if (!vertexSeenIndex[b]) {
                vertexQueue.push_back(b);
                vertexSeenIndex[b] = true;
              }
              const uint16_t &c = index[connectedIndex * 3 + 2];
              if (c != vertexIndex) {
                connectedVertices.push_back(c);
              }
              if (!vertexSeenIndex[c]) {
                vertexQueue.push_back(c);
                vertexSeenIndex[c] = true;
              }

              vertexNormal = addVectors(vertexNormal, faceNormals[connectedIndex]);
            }
            vertexNormal = divideVector(vertexNormal, (float)connectedIndices.size());

            // sort connected vertices clockwise
            sortConnectedVertices(vertex, vertexIndex, vertexNormal, connectedVertices);

            vertexQueue.pop_front();
          }

          islands.push_back(std::move(island));
        }
      }
    }

    // std::cout << "get uvs 3" << std::endl;

    // join islands
    if (islands.size() > 1) {
      const std::vector<uint16_t> &firstIsland = islands[0];
      const uint16_t &a = firstIsland.back();
      const uint16_t &b = adjacentVertices[a].front();
      for (size_t i = 1; i < islands.size(); i++) {
        const std::vector<uint16_t> &island = islands[i];
        const uint16_t &c = island[0];
        const uint16_t &d = adjacentVertices[c].front();

        const MLVec3f faceNormalABC = getTriangleNormal(vertex[a], vertex[b], vertex[c]);
        const MLVec3f faceNormalBDC = getTriangleNormal(vertex[b], vertex[d], vertex[c]);
        // A to C
        {
          std::vector<uint16_t> &connectedVertices = adjacentVertices[a];
          MLVec3f vertexNormal = faceNormalABC;
          for (size_t j = 0; j < connectedVertices.size(); j++) {
            const uint16_t &connectedVertex = connectedVertices[j];
            vertexNormal = addVectors(vertexNormal, faceNormals[connectedVertex]);
          }
          vertexNormal = divideVector(vertexNormal, (float)(connectedVertices.size() + 1));

          connectedVertices.push_back(c);
          sortConnectedVertices(vertex, a, vertexNormal, connectedVertices);
        }
        // B to C, D
        {
          std::vector<uint16_t> &connectedVertices = adjacentVertices[b];
          MLVec3f vertexNormal = addVectors(faceNormalABC, faceNormalBDC);
          for (size_t j = 0; j < connectedVertices.size(); j++) {
            const uint16_t &connectedVertex = connectedVertices[j];
            vertexNormal = addVectors(vertexNormal, faceNormals[connectedVertex]);
          }
          vertexNormal = divideVector(vertexNormal, (float)(connectedVertices.size() + 2));

          connectedVertices.push_back(c);
          connectedVertices.push_back(d);
          sortConnectedVertices(vertex, b, vertexNormal, connectedVertices);
        }
        // C to A, B
        {
          std::vector<uint16_t> &connectedVertices = adjacentVertices[c];
          MLVec3f vertexNormal = addVectors(faceNormalABC, faceNormalBDC);
          for (size_t j = 0; j < connectedVertices.size(); j++) {
            const uint16_t &connectedVertex = connectedVertices[j];
            vertexNormal = addVectors(vertexNormal, faceNormals[connectedVertex]);
          }
          vertexNormal = divideVector(vertexNormal, (float)(connectedVertices.size() + 2));

          connectedVertices.push_back(a);
          connectedVertices.push_back(b);
          sortConnectedVertices(vertex, c, vertexNormal, connectedVertices);
        }
        // D to B
        {
          std::vector<uint16_t> &connectedVertices = adjacentVertices[d];
          MLVec3f vertexNormal = faceNormalBDC;
          for (size_t j = 0; j < connectedVertices.size(); j++) {
            const uint16_t &connectedVertex = connectedVertices[j];
            vertexNormal = addVectors(vertexNormal, faceNormals[connectedVertex]);
          }
          vertexNormal = divideVector(vertexNormal, (float)(connectedVertices.size() + 1));

          connectedVertices.push_back(b);
          sortConnectedVertices(vertex, d, vertexNormal, connectedVertices);
        }
      }
    }

    // std::cout << "get uvs 4" << std::endl;

    uint16_t centerIndex = 0;
    int maxDepth = 1;
    {
      std::deque<std::pair<uint16_t, int>> leaves;
      std::vector<bool> vertexSeenIndex(vertex_count, false);
      for (size_t tryNumLeaves = 2;; tryNumLeaves++) {
        for (uint16_t i = 0; i < (uint16_t)vertex_count; i++) {
          if (adjacentVertices[i].size() <= tryNumLeaves) {
            leaves.push_back(std::pair<uint16_t, int>(i, 0));
            vertexSeenIndex[i] = true;
          }
        }
        if (leaves.size() > 0) {
          break;
        }
      }
      while (leaves.size() > 1) {
        const std::pair<uint16_t, int> &leaf = leaves.front();
        const uint16_t &vertexIndex = leaf.first;
        const int &depth = leaf.second;

        std::vector<uint16_t> &connectedVertices = adjacentVertices[vertexIndex];
        for (size_t i = 0; i < connectedVertices.size(); i++) {
          const uint16_t &nextVertexIndex = connectedVertices[i];
          if (!vertexSeenIndex[nextVertexIndex]) {
            leaves.push_back(std::pair<uint16_t, int>(nextVertexIndex, depth + 1));
            vertexSeenIndex[nextVertexIndex] = true;
          }
        }

        leaves.pop_front();
      }

      const std::pair<uint16_t, int> &lastLeaf = leaves.front();
      centerIndex = lastLeaf.first;
      maxDepth = lastLeaf.second;
    }

    // std::cout << "get uvs 5" << std::endl;

    uvs->resize(vertex_count);
    {
      std::deque<LayerQueueEntry> vertexQueue = {
        LayerQueueEntry{centerIndex, 0, 0, 0, (float)M_PI*2.0f},
      };
      std::vector<bool> vertexSeenIndex(vertex_count, false);
      vertexSeenIndex[centerIndex] = true;
      while (vertexQueue.size() > 0) {
        const LayerQueueEntry &entry = vertexQueue.front();
        const uint16_t &vertexIndex = entry.vertexIndex;
        const unsigned int &depth = entry.depth;
        const size_t &radialIndex = entry.radialIndex;
        const float &radiusStart = entry.radiusStart;
        float radiusEnd = entry.radiusEnd;

        const std::vector<uint16_t> &connectedVertices = adjacentVertices[vertexIndex];
        const float radiusSliceWidth = 1.0f/(float)(connectedVertices.size()-1) * (radiusEnd - radiusStart);
        if (radiusSliceWidth > (float)M_PI/2.0f) {
          radiusEnd = radiusStart + ((radiusEnd - radiusStart) / (radiusSliceWidth / (float)M_PI/2.0f));
        }

        float angle = radiusStart + ((float)radialIndex/(float)(connectedVertices.size()-1)) * (radiusEnd - radiusStart);
        angle = -angle + (float)M_PI/2.0f;

        Uv uv = {
          std::cos(angle) * (float)depth/(float)maxDepth,
          std::sin(angle) * (float)depth/(float)maxDepth
        };
        (*uvs)[vertexIndex] = {
          0.5f + uv.u/2.0f,
          0.5f + uv.v/2.0f
        };
        /* Uv uv2 = {
          0.5f*sqrt( 2.0f + 2.0f*uv.u*SQRT2 + (uv.u*uv.u) - (uv.v*uv.v) ) - 0.5f*sqrt( 2.0f - 2.0f*uv.u*SQRT2 + (uv.u*uv.u) - (uv.v*uv.v) ),
          0.5f*sqrt( 2.0f + 2.0f*uv.v*SQRT2 - (uv.u*uv.u) + (uv.v*uv.v) ) - 0.5f*sqrt( 2.0f - 2.0f*uv.v*SQRT2 - (uv.u*uv.u) + (uv.v*uv.v) )
        };
        (*uvs)[vertexIndex] = {
          0.5f + uv2.u/2.0f,
          0.5f + uv2.v/2.0f
        }; */

        for (size_t j = 0; j < connectedVertices.size(); j++) {
          const uint16_t connectedVertex = connectedVertices[j];
          if (!vertexSeenIndex[connectedVertex]) {
            vertexQueue.push_back(LayerQueueEntry{
              connectedVertex,
              depth + 1,
              j,
              radiusStart + (float)j / (float)(connectedVertices.size()-1) * (radiusEnd - radiusStart),
              radiusStart + (float)(j+1) / (float)(connectedVertices.size()-1) * (radiusEnd - radiusStart)
            });
            vertexSeenIndex[connectedVertex] = true;
          }
        }

        vertexQueue.pop_front();
      }
    }

    // std::cout << "get uvs 6" << std::endl;
  } else {
    uvs->clear();
  }
}

NAN_METHOD(MLContext::PostPollEvents) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(Local<Object>::Cast(info[0]));
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[1]));
  NATIVEwindow *window = application_context.window;

  GLuint fbo = info[2]->Uint32Value();

  if (meshInfoRequestPending) {
    MLResult result = MLMeshingGetMeshInfoResult(meshTracker, meshInfoRequestHandle, &meshInfo);
    if (result == MLResult_Ok) {
      uint32_t dataCount = meshInfo.data_count;

      meshRequestNewMap.clear();
      meshRequestRemovedMap.clear();
      meshRequestUnchangedMap.clear();
      meshRequestCentersMap.clear();
      for (uint32_t i = 0; i < dataCount; i++) {
        const MLMeshingBlockInfo &meshBlockInfo = meshInfo.data[i];
        const MLMeshingMeshState &state = meshBlockInfo.state;
        const MLMeshingExtents &extents = meshBlockInfo.extents;
        MLMeshingBlockRequest &meshBlockRequest = meshBlockRequests[i];
        meshBlockRequest.id = meshBlockInfo.id;
        // meshBlockRequest.level = MLMeshingLOD_Minimum;
        meshBlockRequest.level = MLMeshingLOD_Medium;
        // meshBlockRequest.level = MLMeshingLOD_Maximum;

        const std::string &id = id2String(meshBlockInfo.id);
        meshRequestNewMap[id] = (state == MLMeshingMeshState_New);
        meshRequestRemovedMap[id] = (state == MLMeshingMeshState_Deleted);
        meshRequestUnchangedMap[id] = (state == MLMeshingMeshState_Unchanged);
        meshRequestCentersMap[id] = extents.center;
      }
      numMeshBlockRequests = dataCount;

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

      std::vector<std::vector<Uv>> uvs(dataCount);

      for (uint32_t i = 0; i < dataCount; i++) {
        MLMeshingBlockMesh &blockMesh = blockMeshes[i];
        const std::string &id = id2String(blockMesh.id);

        if (!meshRequestRemovedMap[id]) {
          MeshBuffer *meshBuffer;
          auto iter = meshBuffers.find(id);
          if (iter != meshBuffers.end()) {
            meshBuffer = &iter->second;
          } else {
            GLuint buffers[4];
            glGenBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
            meshBuffers[id] = MeshBuffer(buffers[0], buffers[1], buffers[2], buffers[3]);
            meshBuffer = &meshBuffers[id];
          }

          std::vector<Uv> &localUvs = uvs[i];
          getUvs(blockMesh.vertex, blockMesh.vertex_count, blockMesh.index, blockMesh.index_count, &localUvs);

          // std::cout << "set buffers 1" << std::endl;

          meshBuffer->setBuffers((float *)(&blockMesh.vertex->values), blockMesh.vertex_count * 3, (float *)(&blockMesh.normal->values), blockMesh.index, blockMesh.index_count, &localUvs, meshRequestNewMap[id], meshRequestUnchangedMap[id], meshRequestCentersMap[id]);

          // std::cout << "set buffers 2" << std::endl;
        } else {
          auto iter = meshBuffers.find(id);
          if (iter != meshBuffers.end()) {
            MeshBuffer *meshBuffer = &iter->second;
            GLuint buffers[] = {
              meshBuffer->positionBuffer,
              meshBuffer->normalBuffer,
              meshBuffer->uvBuffer,
              meshBuffer->indexBuffer,
            };
            glDeleteBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
            if (meshBuffer->texture) {
              glDeleteTextures(1, &meshBuffer->texture);
            }
            meshBuffers.erase(iter);
          }
        }
      }

      if (cameraMeshEnabled) {
        MLHandle output;
        MLResult result = MLCameraGetPreviewStream(&output);
        if (result == MLResult_Ok) {
          ANativeWindowBuffer_t *aNativeWindowBuffer = (ANativeWindowBuffer_t *)output;

          EGLImageKHR yuv_img = eglCreateImageKHR(window->display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, (EGLClientBuffer)(void*)output, nullptr);

          glActiveTexture(GL_TEXTURE1);
          glBindTexture(GL_TEXTURE_EXTERNAL_OES, mlContext->cameraInTexture);
          glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, yuv_img);
          glActiveTexture(GL_TEXTURE0);

          {
            GLuint error = glGetError();
            if (error) {
              std::cout << "error 1 " << error << std::endl;
            }
          }

          for (std::map<std::string, MeshBuffer>::iterator iter = meshBuffers.begin(); iter != meshBuffers.end(); iter++) {
            MeshBuffer &meshBuffer = iter->second;

            {
              glBindVertexArray(mlContext->cameraMeshVao1);
              glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->cameraMeshFbo);
              if (!meshBuffer.texture) {
                glGenTextures(1, &meshBuffer.texture);
                glBindTexture(GL_TEXTURE_2D, meshBuffer.texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, CAMERA_SIZE[0], CAMERA_SIZE[1], 0, GL_RED, GL_FLOAT, NULL);
                
                {
                  GLuint error = glGetError();
                  if (error) {
                    std::cout << "error 2 " << error << std::endl;
                  }
                }
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              } else {
                glBindTexture(GL_TEXTURE_2D, meshBuffer.texture);
              }
              
              {
                  GLuint error = glGetError();
                  if (error) {
                    std::cout << "error 3 " << error << std::endl;
                  }
                }
              
              glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, meshBuffer.texture, 0);
              glUseProgram(mlContext->cameraMeshProgram1);

              const MLGraphicsVirtualCameraInfo &cameraInfo = mlContext->virtual_camera_array.virtual_cameras[0];
              const MLTransform &transform = cameraInfo.transform;
              const MLMat4f &modelView = invertMatrix(composeMatrix(transform.position, transform.rotation));
              glUniformMatrix4fv(mlContext->cameraMeshModelViewMatrixLocation1, 1, false, modelView.matrix_colmajor);

              const MLMat4f &projection = cameraInfo.projection;
              glUniformMatrix4fv(mlContext->cameraMeshProjectionMatrixLocation1, 1, false, projection.matrix_colmajor);

              glBindBuffer(GL_ARRAY_BUFFER, meshBuffer.positionBuffer);
              glEnableVertexAttribArray(mlContext->cameraMeshPositionLocation1);
              glVertexAttribPointer(mlContext->cameraMeshPositionLocation1, 3, GL_FLOAT, GL_FALSE, 0, 0);

              // glBindBuffer(GL_ARRAY_BUFFER, meshBuffer.indexBuffer);
              // glEnableVertexAttribArray(mlContext->cameraMeshIndexLocation1);
              // glVertexAttribIPointer(mlContext->cameraMeshIndexLocation1, 1, GL_UNSIGNED_SHORT, 0, 0);

              glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshBuffer.indexBuffer);

              glViewport(0, 0, CAMERA_SIZE[0], CAMERA_SIZE[1]);
              glClearColor(0, 0, 0, 1); // XXX
              glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
              glDrawElements(GL_TRIANGLES, meshBuffer.numIndices, GL_UNSIGNED_SHORT, 0);
            }

            {
              glBindVertexArray(mlContext->cameraMeshVao2);
              glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->cameraMeshFbo2);
              if (!meshBuffer.texture2) {
                glGenTextures(1, &meshBuffer.texture2);
                glBindTexture(GL_TEXTURE_2D, meshBuffer.texture2);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, CAMERA_SIZE[0], CAMERA_SIZE[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              } else {
                glBindTexture(GL_TEXTURE_2D, meshBuffer.texture2);
              }
              glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, meshBuffer.texture2, 0);
              glUseProgram(mlContext->cameraMeshProgram2);

              const MLGraphicsVirtualCameraInfo &cameraInfo = mlContext->virtual_camera_array.virtual_cameras[0];
              const MLTransform &transform = cameraInfo.transform;
              const MLMat4f &modelView = invertMatrix(composeMatrix(transform.position, transform.rotation));
              glUniformMatrix4fv(mlContext->cameraMeshModelViewMatrixLocation2, 1, false, modelView.matrix_colmajor);

              const MLMat4f &projection = cameraInfo.projection;
              glUniformMatrix4fv(mlContext->cameraMeshProjectionMatrixLocation2, 1, false, projection.matrix_colmajor);

              glActiveTexture(GL_TEXTURE0);
              glBindTexture(GL_TEXTURE_2D, meshBuffer.texture);
              glUniform1i(mlContext->cameraMeshPrevStageTextureLocation2, 0);

              glActiveTexture(GL_TEXTURE1);
              glBindTexture(GL_TEXTURE_EXTERNAL_OES, mlContext->cameraInTexture);
              glUniform1i(mlContext->cameraMeshCameraInTextureLocation2, 1);

              glBindBuffer(GL_ARRAY_BUFFER, meshBuffer.positionBuffer);
              glEnableVertexAttribArray(mlContext->cameraMeshPositionLocation2);
              glVertexAttribPointer(mlContext->cameraMeshPositionLocation2, 3, GL_FLOAT, GL_FALSE, 0, 0);

              glBindBuffer(GL_ARRAY_BUFFER, meshBuffer.uvBuffer);
              glEnableVertexAttribArray(mlContext->cameraMeshUvLocation2);
              glVertexAttribPointer(mlContext->cameraMeshUvLocation2, 2, GL_FLOAT, GL_FALSE, 0, 0);

              glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshBuffer.indexBuffer);

              glViewport(0, 0, CAMERA_SIZE[0], CAMERA_SIZE[1]);
              glClearColor(0.2, 0.2, 0.2, 1); // XXX
              glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
              glClearColor(0, 0, 0, 1);
              glDrawElements(GL_TRIANGLES, meshBuffer.numIndices, GL_UNSIGNED_SHORT, 0);
            }
          }
          
          {
            GLuint error = glGetError();
            if (error) {
              std::cout << "error 4 " << error << std::endl;
            }
          }

          eglDestroyImageKHR(window->display, yuv_img);
        } else {
          ML_LOG(Error, "%s: failed to get camera preview stream %x", application_name, result);
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
      glActiveTexture(gl->activeTexture);

      std::for_each(meshers.begin(), meshers.end(), [&](MLMesher *m) {
        m->Poll();
      });

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
}

}

Handle<Object> makeMl() {
  Nan::EscapableHandleScope scope;
  return scope.Escape(ml::MLContext::Initialize(Isolate::GetCurrent()));
}

#endif

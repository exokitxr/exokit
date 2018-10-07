#if defined(LUMIN)

#include <magicleap.h>
#include <ml-math.h>
#include <ml-ffmpeg.h>
#include <uv.h>
#include <iostream>

using namespace v8;
using namespace std;

namespace ml {

const char application_name[] = "com.exokit.app";
constexpr int CAMERA_SIZE[] = {1920, 1080};
constexpr int MESH_TEXTURE_SIZE[] = {512, 512};
constexpr int CAMERA_STREAM_SIZE[] = {CAMERA_SIZE[0]/2, CAMERA_SIZE[1]/2};

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
Nan::Persistent<Function> mlCameraMesherConstructor;

bool cameraConnected = false;
std::thread cameraThread;
// std::mutex cameraRequestsMutex;
std::vector<CameraRequest *> cameraRequests;
uv_async_t cameraAsync;
// bool cameraConvertPending = false;
// uint8_t cameraRequestRgb[CAMERA_SIZE[0]/2 * CAMERA_SIZE[1]/2 * 3];
// uint8_t *cameraRequestJpeg;
// size_t cameraRequestSize;
// uv_sem_t cameraConvertSem;
// uv_async_t cameraConvertAsync;

std::deque<CameraMeshPreviewRequest> cameraMeshPreviewRequests;
std::deque<CameraPosition> cameraPositions;

std::vector<MLStream *> cameraStreams;
std::vector<CameraStreamRequest *> cameraStreamRequests;
// std::vector<CameraStreamResponse *> cameraStreamResponses;

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
std::map<std::string, MeshRequestSpec> meshRequestSpecMap;
MLHandle meshRequestHandle;
MLMeshingMesh mesh;
bool meshRequestsPending = false;
bool meshRequestPending = false;

std::map<std::string, MeshBuffer> meshBuffers;

std::vector<MLCameraMesher *> cameraMeshers;

// std::deque<std::pair<std::string, GLuint>> textureEncodePbos;
std::deque<TextureEncodeRequestEntry *> textureEncodeRequestQueue;
std::deque<TextureEncodeResponseEntry *> textureEncodeResponseQueue;
std::mutex textureEncodeRequestMutex;
std::mutex textureEncodeResponseMutex;
uv_sem_t textureEncodeSem;
uv_async_t textureEncodeAsync;

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

class PixelCb {
public:
  PixelCb(const std::string &id, int width, int height, GLuint texture, bool cleanup, std::function<void(GLuint, uint8_t *)> fn) : id(id), width(width), height(height), texture(texture), cleanup(cleanup), fn(fn) {}
  PixelCb() = default;
  PixelCb(const PixelCb &pixelCb) = default;
  PixelCb(PixelCb &&pixelCb) = default;
  PixelCb &operator=(const PixelCb &pixelCb) = default;
  PixelCb &operator=(PixelCb &&pixelCb) = default;

// protected:
  std::string id;
  int width;
  int height;
  GLuint texture;
  bool cleanup;
  std::function<void(GLuint, uint8_t *)> fn;

  GLuint fbo;
  GLuint pbo;
};

std::deque<PixelCb> pixelCbs;
PixelCb currentGetPixels;
bool currentGetPixelsValid = false;
void tickGetPixelsRequest() {
  WebGLRenderingContext *gl = application_context.gl;

  if (!currentGetPixelsValid && pixelCbs.size() > 0) {
    PixelCb &pixelCb = pixelCbs.front();

    glGenFramebuffers(1, &pixelCb.fbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, pixelCb.fbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pixelCb.texture, 0);

    glGenBuffers(1, &pixelCb.pbo);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelCb.pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, pixelCb.width * pixelCb.height * 4, 0, GL_STREAM_READ);
    glReadPixels(0, 0, pixelCb.width, pixelCb.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);

    currentGetPixels = std::move(pixelCb);
    currentGetPixelsValid = true;
    pixelCbs.pop_front();

    if (gl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
    } else {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->defaultFramebuffer);
    }
    if (gl->HasBufferBinding(GL_PIXEL_PACK_BUFFER)) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->GetBufferBinding(GL_PIXEL_PACK_BUFFER));
    } else {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }
  }
}
void tickGetPixelsResponse() {
  WebGLRenderingContext *gl = application_context.gl;

  if (currentGetPixelsValid) {
    glBindBuffer(GL_PIXEL_PACK_BUFFER, currentGetPixels.pbo);

    uint8_t *textureDataRgb = (uint8_t *)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, currentGetPixels.width * currentGetPixels.height * 4, GL_MAP_READ_BIT);

    currentGetPixels.fn(currentGetPixels.pbo, textureDataRgb);

    glDeleteFramebuffers(1, &currentGetPixels.fbo);
    if (currentGetPixels.cleanup) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, currentGetPixels.pbo);
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      glDeleteBuffers(1, &currentGetPixels.pbo);
    }

    currentGetPixelsValid = false;

    if (gl->HasBufferBinding(GL_PIXEL_PACK_BUFFER)) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->GetBufferBinding(GL_PIXEL_PACK_BUFFER));
    } else {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }
  }

  tickGetPixelsRequest();
}
void getPixels(const std::string &id, int width, int height, GLuint texture, bool cleanup, std::function<void(GLuint, uint8_t *)> fn) {
  pixelCbs.push_back(PixelCb(id, width, height, texture, cleanup, fn));

  tickGetPixelsRequest();
}
void cancelGetPixels(const std::string &id) {
  if (currentGetPixelsValid && currentGetPixels.id == id) {
    glDeleteFramebuffers(1, &currentGetPixels.fbo);
    glDeleteBuffers(1, &currentGetPixels.pbo);
    currentGetPixelsValid = false;
  }
  auto match = std::find_if(pixelCbs.begin(), pixelCbs.end(), [&](const PixelCb &pixelCb) -> bool {
    return pixelCb.id == id;
  });
  if (match != pixelCbs.end()) {
    // XXX delete
    pixelCbs.erase(match);
  }
}

// MeshBuffer

void MeshBuffer::setBuffers(const std::string &id, float *positions, uint32_t numPositions, float *normals, uint16_t *indices, uint16_t numIndices, std::vector<MLVec3f> *vertices, std::vector<Uv> *uvs, const MLVec3f &center, bool isNew, bool isUnchanged) {
  WebGLRenderingContext *gl = application_context.gl;

  glBindBuffer(GL_ARRAY_BUFFER, this->positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(positions[0]), positions, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
  glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(normals[0]), normals, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, this->indexBuffer);
  glBufferData(GL_ARRAY_BUFFER, numIndices * sizeof(indices[0]), indices, GL_DYNAMIC_DRAW);

  this->positions = positions;
  this->numPositions = numPositions;
  this->normals = normals;
  this->indices = indices;
  this->numIndices = numIndices;
  this->center = center;
  this->isNew = isNew;
  this->isUnchanged = isUnchanged;

  if (vertices->size() > 0 && uvs->size() > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices->size() * 3 * sizeof(float), vertices->data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, this->uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs->size() * 2 * sizeof(float), uvs->data(), GL_DYNAMIC_DRAW);

    this->vertices = std::move(*vertices);
    this->uvs = std::move(*uvs);
  }

  /* {
    auto match = std::find_if(textureEncodePbos.begin(), textureEncodePbos.end(), [&](std::pair<std::string, GLuint> &textureEncodePbo) -> bool {
      return textureEncodePbo.first == id;
    });
    if (match != textureEncodePbos.end()) {
      std::pair<std::string, GLuint> &textureEncodePbo = *match;
      GLuint &pbo = textureEncodePbo.second;

      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
      glDeleteBuffers(1, &pbo);
      if (gl->HasBufferBinding(GL_PIXEL_PACK_BUFFER)) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->GetBufferBinding(GL_PIXEL_PACK_BUFFER));
      } else {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
      }

      textureEncodePbos.erase(match);
    }
  } */
  {
    cancelGetPixels(id);
  }
  {
    TextureEncodeRequestEntry *entry;
    {
      std::lock_guard<mutex> lock(textureEncodeRequestMutex);
      auto match = std::find_if(textureEncodeRequestQueue.begin(), textureEncodeRequestQueue.end(), [&](TextureEncodeRequestEntry *textureEncodeRequest) -> bool {
        return textureEncodeRequest->type == TextureEncodeEntryType::MESH_BUFFER && textureEncodeRequest->id == id;
      });
      if (match != textureEncodeRequestQueue.end()) {
        entry = *match;

        textureEncodeRequestQueue.erase(match);
      } else {
        entry = nullptr;
      }
    }
    if (entry) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, entry->pbo);
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      glDeleteBuffers(1, &entry->pbo);
      if (gl->HasBufferBinding(GL_PIXEL_PACK_BUFFER)) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->GetBufferBinding(GL_PIXEL_PACK_BUFFER));
      } else {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
      }

      delete entry;
    }
  }
  {
    TextureEncodeResponseEntry *entry;
    {
      std::lock_guard<mutex> lock(textureEncodeResponseMutex);
      auto match = std::find_if(textureEncodeResponseQueue.begin(), textureEncodeResponseQueue.end(), [&](TextureEncodeResponseEntry *textureEncodeResponse) -> bool {
        return textureEncodeResponse->type == TextureEncodeEntryType::MESH_BUFFER && textureEncodeResponse->id == id;
      });
      if (match != textureEncodeResponseQueue.end()) {
        entry = *match;

        textureEncodeResponseQueue.erase(match);
      } else {
        entry = nullptr;
      }
    }
    if (entry) {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, entry->pbo);
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      glDeleteBuffers(1, &entry->pbo);
      if (gl->HasBufferBinding(GL_PIXEL_PACK_BUFFER)) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->GetBufferBinding(GL_PIXEL_PACK_BUFFER));
      } else {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
      }
      if (entry->result) {
        SjpegFreeBuffer(entry->result);
      }
      delete entry;
    }
  }
}
void MeshBuffer::beginRenderCameraAll() {
  MLContext *mlContext = application_context.mlContext;

  glBindVertexArray(mlContext->cameraMeshVao2);
  glUseProgram(mlContext->cameraMeshProgram2);

  glViewport(0, 0, MESH_TEXTURE_SIZE[0], MESH_TEXTURE_SIZE[1]);
}
void MeshBuffer::beginRenderCamera() {
  MLContext *mlContext = application_context.mlContext;

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->fbo);

  glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
  glVertexAttribPointer(mlContext->cameraMeshPositionLocation2, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(mlContext->cameraMeshPositionLocation2);

  glBindBuffer(GL_ARRAY_BUFFER, this->uvBuffer);
  glVertexAttribPointer(mlContext->cameraMeshUvLocation2, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(mlContext->cameraMeshUvLocation2);
}
void MeshBuffer::renderCamera(const CameraMeshPreviewRequest &cameraMeshPreviewRequest) {
  MLContext *mlContext = application_context.mlContext;

  glUniformMatrix4fv(mlContext->cameraMeshModelViewMatrixLocation2, 1, false, cameraMeshPreviewRequest.modelViewInverse.matrix_colmajor);
  glUniformMatrix4fv(mlContext->cameraMeshProjectionMatrixLocation2, 1, false, cameraMeshPreviewRequest.projection.matrix_colmajor);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, cameraMeshPreviewRequest.texture);
  glUniform1i(mlContext->cameraMeshCameraSnapshotTextureLocation, 0);

  glDrawArrays(GL_TRIANGLES, 0, this->vertices.size() * 3);

  if (this->textureData) {
    SjpegFreeBuffer(this->textureData);
    this->textureData = nullptr;
    this->textureDataSize = 0;
  }
  this->textureDirty = true;
}
void MeshBuffer::readPixels(const std::string &id, std::function<void(GLuint, uint8_t *)> fn) {
  // glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbo);

  // std::cout << "mesh buffer get pixels 1 1 " << texture << std::endl;

  getPixels(id, MESH_TEXTURE_SIZE[0], MESH_TEXTURE_SIZE[1], texture, false, fn);

  // std::cout << "mesh buffer get pixels 1 2" << std::endl;

  this->textureDirty = false;
}
void MeshBuffer::endRenderCamera() {
  WebGLRenderingContext *gl = application_context.gl;

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
  if (gl->HasBufferBinding(GL_PIXEL_PACK_BUFFER)) {
    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->GetBufferBinding(GL_PIXEL_PACK_BUFFER));
  } else {
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
  }
  if (gl->HasTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D)) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  if (gl->HasTextureBinding(GL_TEXTURE1, GL_TEXTURE_EXTERNAL_OES)) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, gl->GetTextureBinding(GL_TEXTURE1, GL_TEXTURE_EXTERNAL_OES));
  } else {
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
  }
  glActiveTexture(gl->activeTexture);
}

TextureEncodeRequestEntry::TextureEncodeRequestEntry(TextureEncodeEntryType type, const std::string id, int width, int height, GLuint pbo, uint8_t *data) :
  type(type),
  id(id),
  width(width),
  height(height),
  pbo(pbo),
  data(data)
  {}

TextureEncodeResponseEntry::TextureEncodeResponseEntry(TextureEncodeEntryType type, const std::string id, GLuint pbo, uint8_t *result, size_t resultSize) :
  type(type),
  id(id),
  pbo(pbo),
  result(result),
  resultSize(resultSize)
  {}

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

      if (!meshRequestSpecMap[id].isRemoved) {
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

          Local<Object> vertexBuffer = Nan::New<Object>();
          vertexBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.vertexBuffer));
          obj->Set(JS_STR("vertexBuffer"), vertexBuffer);
          Local<Float32Array> vertexArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.vertices.data(), meshBuffer.vertices.size() * 3 * sizeof(float)), 0, meshBuffer.vertices.size() * 3);
      obj->Set(JS_STR("vertexArray"), vertexArray);
          obj->Set(JS_STR("vertexCount"), JS_INT((uint32_t)meshBuffer.vertices.size() * 3));

          Local<Object> uvBuffer = Nan::New<Object>();
          uvBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.uvBuffer));
          obj->Set(JS_STR("uvBuffer"), uvBuffer);
          Local<Float32Array> uvArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.uvs.data(), meshBuffer.uvs.size() * 2 * sizeof(float)), 0, meshBuffer.uvs.size() * 2);
      obj->Set(JS_STR("uvArray"), uvArray);
          obj->Set(JS_STR("uvCount"), JS_INT((uint32_t)meshBuffer.uvs.size() * 2));

          Local<Object> indexBuffer = Nan::New<Object>();
          indexBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.indexBuffer));
          obj->Set(JS_STR("indexBuffer"), indexBuffer);
          Local<Uint16Array> indexArray = Uint16Array::New(ArrayBuffer::New(Isolate::GetCurrent(), meshBuffer.indices, meshBuffer.numIndices * sizeof(uint16_t)), 0, meshBuffer.numIndices);
          obj->Set(JS_STR("indexArray"), indexArray);
          obj->Set(JS_STR("count"), JS_INT(meshBuffer.numIndices));

          Local<Object> textureVal = Nan::New<Object>();
          textureVal->Set(JS_STR("id"), JS_INT(meshBuffer.texture));
          obj->Set(JS_STR("texture"), textureVal);

          Local<Value> textureDataVal;
          if (meshBuffer.textureData) {
            Local<Uint8Array> textureDataArray = Uint8Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.textureData, meshBuffer.textureDataSize), 0, meshBuffer.textureDataSize);
            textureDataVal = textureDataArray;
          } else {
            textureDataVal = Nan::Null();
          }
          obj->Set(JS_STR("textureData"), textureDataVal);

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

// MLCameraMesher

MLCameraMesher::MLCameraMesher() {}

MLCameraMesher::~MLCameraMesher() {}

NAN_METHOD(MLCameraMesher::New) {
  MLCameraMesher *mlCameraMesher = new MLCameraMesher();
  Local<Object> mlCameraMesherObj = info.This();
  mlCameraMesher->Wrap(mlCameraMesherObj);

  Nan::SetAccessor(mlCameraMesherObj, JS_STR("onmesh"), OnMeshGetter, OnMeshSetter);

  info.GetReturnValue().Set(mlCameraMesherObj);

  cameraMeshers.push_back(mlCameraMesher);
}

Local<Function> MLCameraMesher::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("MLCameraMesher"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "destroy", Destroy);

  Local<Function> ctorFn = ctor->GetFunction();

  return scope.Escape(ctorFn);
}

NAN_GETTER(MLCameraMesher::OnMeshGetter) {
  // Nan::HandleScope scope;
  MLCameraMesher *mlCameraMesher = ObjectWrap::Unwrap<MLCameraMesher>(info.This());

  Local<Function> cb = Nan::New(mlCameraMesher->cb);
  info.GetReturnValue().Set(cb);
}

NAN_SETTER(MLCameraMesher::OnMeshSetter) {
  // Nan::HandleScope scope;
  MLCameraMesher *mlCameraMesher = ObjectWrap::Unwrap<MLCameraMesher>(info.This());

  if (value->IsFunction()) {
    Local<Function> localCb = Local<Function>::Cast(value);
    mlCameraMesher->cb.Reset(localCb);
  } else {
    Nan::ThrowError("MLCameraMesher::OnMeshSetter: invalid arguments");
  }
}

void MLCameraMesher::Poll() {
  if (!this->cb.IsEmpty()) {
    Local<Object> asyncObject = Nan::New<Object>();
    AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "MLCameraMesher::Poll()");

    Local<Array> array = Nan::New<Array>(meshBuffers.size());
    uint32_t numResults = 0;
    for (auto iter = meshBuffers.begin(); iter != meshBuffers.end(); iter++) {
      const std::string &id = iter->first;
      const MeshBuffer &meshBuffer = iter->second;

      Local<Object> obj = Nan::New<Object>();
      obj->Set(JS_STR("id"), JS_STR(id));

      Local<Object> vertexBuffer = Nan::New<Object>();
      vertexBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.vertexBuffer));
      obj->Set(JS_STR("vertexBuffer"), vertexBuffer);
      Local<Float32Array> vertexArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.vertices.data(), meshBuffer.vertices.size() * 3 * sizeof(float)), 0, meshBuffer.vertices.size() * 3);
      obj->Set(JS_STR("vertexArray"), vertexArray);
      obj->Set(JS_STR("vertexCount"), JS_INT((uint32_t)meshBuffer.vertices.size() * 3));

      Local<Object> uvBuffer = Nan::New<Object>();
      uvBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.uvBuffer));
      obj->Set(JS_STR("uvBuffer"), uvBuffer);
      Local<Float32Array> uvArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.uvs.data(), meshBuffer.uvs.size() * 2 * sizeof(float)), 0, meshBuffer.uvs.size() * 2);
      obj->Set(JS_STR("uvArray"), uvArray);
      obj->Set(JS_STR("uvCount"), JS_INT((uint32_t)meshBuffer.uvs.size() * 2));

      Local<Object> textureObj = Nan::New<Object>();
      textureObj->Set(JS_STR("id"), JS_INT(meshBuffer.texture));
      obj->Set(JS_STR("texture"), textureObj);

      Local<Value> textureDataVal;
      if (meshBuffer.textureData) {
        Local<Uint8Array> textureDataArray = Uint8Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.textureData, meshBuffer.textureDataSize), 0, meshBuffer.textureDataSize);
        textureDataVal = textureDataArray;
      } else {
        textureDataVal = Nan::Null();
      }
      obj->Set(JS_STR("textureData"), textureDataVal);

      array->Set(numResults++, obj);
    }

    Local<Function> cbFn = Nan::New(this->cb);
    Local<Value> argv[] = {
      array,
    };
    asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);
  }
}

void MLCameraMesher::Poll(const std::vector<std::string> &meshBufferIds) {
  if (!this->cb.IsEmpty()) {
    Local<Object> asyncObject = Nan::New<Object>();
    AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "MLCameraMesher::Poll(meshBufferIds)");

    Local<Array> array = Nan::New<Array>(meshBufferIds.size());
    uint32_t numResults = 0;
    for (auto iter = meshBufferIds.begin(); iter != meshBufferIds.end(); iter++) {
      const std::string &id = *iter;
      const MeshBuffer &meshBuffer = meshBuffers[id];

      Local<Object> obj = Nan::New<Object>();
      obj->Set(JS_STR("id"), JS_STR(id));

      Local<Object> vertexBuffer = Nan::New<Object>();
      vertexBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.vertexBuffer));
      obj->Set(JS_STR("vertexBuffer"), vertexBuffer);
      Local<Float32Array> vertexArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.vertices.data(), meshBuffer.vertices.size() * 3 * sizeof(float)), 0, meshBuffer.vertices.size() * 3);
      obj->Set(JS_STR("vertexArray"), vertexArray);
      obj->Set(JS_STR("vertexCount"), JS_INT((uint32_t)meshBuffer.vertices.size() * 3));

      Local<Object> uvBuffer = Nan::New<Object>();
      uvBuffer->Set(JS_STR("id"), JS_INT(meshBuffer.uvBuffer));
      obj->Set(JS_STR("uvBuffer"), uvBuffer);
      Local<Float32Array> uvArray = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.uvs.data(), meshBuffer.uvs.size() * 2 * sizeof(float)), 0, meshBuffer.uvs.size() * 2);
      obj->Set(JS_STR("uvArray"), uvArray);
      obj->Set(JS_STR("uvCount"), JS_INT((uint32_t)meshBuffer.uvs.size() * 2));

      Local<Object> textureObj = Nan::New<Object>();
      textureObj->Set(JS_STR("id"), JS_INT(meshBuffer.texture));
      obj->Set(JS_STR("texture"), textureObj);

      Local<Value> textureDataVal;
      if (meshBuffer.textureData) {
        Local<Uint8Array> textureDataArray = Uint8Array::New(ArrayBuffer::New(Isolate::GetCurrent(), (void *)meshBuffer.textureData, meshBuffer.textureDataSize), 0, meshBuffer.textureDataSize);
        textureDataVal = textureDataArray;
      } else {
        textureDataVal = Nan::Null();
      }
      obj->Set(JS_STR("textureData"), textureDataVal);

      array->Set(numResults++, obj);
    }

    Local<Function> cbFn = Nan::New(this->cb);
    Local<Value> argv[] = {
      array,
    };
    asyncResource.MakeCallback(cbFn, sizeof(argv)/sizeof(argv[0]), argv);
  }
}

NAN_METHOD(MLCameraMesher::Destroy) {
  MLCameraMesher *mlCameraMesher = ObjectWrap::Unwrap<MLCameraMesher>(info.This());

  cameraMeshers.erase(std::remove_if(cameraMeshers.begin(), cameraMeshers.end(), [&](MLCameraMesher *m) -> bool {
    if (m == mlCameraMesher) {
      delete m;
      return true;
    } else {
      return false;
    }
  }), cameraMeshers.end());
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


MLResult connectCamera() {
  MLResult result = MLCameraConnect();
  if (result == MLResult_Ok) {
    cameraConnected = true;

    /* cameraThread = std::thread([]() -> void {
      for (;;) {
        uv_sem_wait(&cameraConvertSem);

        if (cameraConnected) {
          cameraRequestSize = SjpegCompress(cameraRequestRgb, CAMERA_SIZE[0]/2, CAMERA_SIZE[1]/2, 50.0f, &cameraRequestJpeg);

          uv_async_send(&cameraConvertAsync);
        } else {
          break;
        }
      }
    }); */
    cameraThread = std::thread([]() -> void {
      for (;;) {
        uv_sem_wait(&textureEncodeSem);

        if (cameraConnected) {
          TextureEncodeRequestEntry *requestEntry;
          {
            std::lock_guard<mutex> lock(textureEncodeRequestMutex);
            if (textureEncodeRequestQueue.size() > 0) {
              requestEntry = textureEncodeRequestQueue.front();
              textureEncodeRequestQueue.pop_front();
            } else {
              requestEntry = nullptr;
            }
          }

          if (requestEntry) {
            std::vector<uint8_t> rgb(requestEntry->width * requestEntry->height * 3);
            uint8_t *src = requestEntry->data;
            uint8_t *dst = rgb.data();
            for (int i = 0; i < requestEntry->width * requestEntry->height; i++) {
              uint8_t b = src[0];
              uint8_t g = src[1];
              uint8_t r = src[2];
              dst[0] = r;
              dst[1] = g;
              dst[2] = b;
              src += 4;
              dst += 3;
            }
            uint8_t *result = nullptr;
            size_t resultSize = SjpegCompress(rgb.data(), requestEntry->width, requestEntry->height, 50.0f, &result);

            if (resultSize == 0) {
              ML_LOG(Error, "%s: failed to encode jpeg: %x", application_name, resultSize);
            }

            TextureEncodeResponseEntry *responseEntry = new TextureEncodeResponseEntry(requestEntry->type, requestEntry->id, requestEntry->pbo, result, resultSize);
            {
              std::lock_guard<mutex> lock(textureEncodeResponseMutex);
              textureEncodeResponseQueue.push_back(responseEntry);
            }

            uv_async_send(&textureEncodeAsync);

            delete requestEntry;
          }
        } else {
          break;
        }
      }
    });
  }
  return result;
}

MLContext::MLContext() : window(nullptr), gl(nullptr), position{0, 0, 0}, rotation{0, 0, 0, 1} {}

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
  Nan::SetMethod(ctorFn, "RequestCameraMeshing", RequestCameraMeshing);
  Nan::SetMethod(ctorFn, "RequestCameraStream", RequestCameraStream);
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

  if (cameraMeshEnabled || cameraStreams.size() > 0) {
    MLContext *mlContext = application_context.mlContext;

    const MLGraphicsVirtualCameraInfo &cameraInfo = mlContext->virtual_camera_array.virtual_cameras[0];
    const MLTransform &transform = cameraInfo.transform;

    const MLVec3f &position = transform.position;
    const MLQuaternionf &rotation = transform.rotation;
    const MLMat4f &modelView = composeMatrix(
      addVectors(position, applyVectorQuaternion(MLVec3f{-0.03f, 0.0f, 0.0f}, rotation)),
      rotation
    );
    const MLMat4f &modelViewInverse = invertMatrix(modelView);
    const MLMat4f &projection = cameraInfo.projection;
    const MLFrustumf &frustum = makeFrustumFromMatrix(multiplyMatrices(projection, modelViewInverse));
    const milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) + CAMERA_PREVIEW_DELAY;

    cameraMeshPreviewRequests.push_back(CameraMeshPreviewRequest{position, rotation, modelViewInverse, projection, frustum, 0, ms});
  }
}

/* void RunCameraConvertInMainThread(uv_async_t *handle) {
  Nan::HandleScope scope;

  std::for_each(cameraRequests.begin(), cameraRequests.end(), [&](CameraRequest *c) {
    c->Set(CAMERA_SIZE[0]/2, CAMERA_SIZE[1]/2, cameraRequestJpeg, cameraRequestSize);
    c->Poll();
  });

  if (cameraRequestSize > 0) {
    SjpegFreeBuffer(cameraRequestJpeg);
  }

  cameraConvertPending = false;
} */

void RunTextureEncodeInMainThread(uv_async_t *handle) {
  Nan::HandleScope scope;

  WebGLRenderingContext *gl = application_context.gl;

  std::vector<std::string> pollMeshBufferIdList;
  for (;;) {
    TextureEncodeResponseEntry *entry;
    {
      std::lock_guard<mutex> lock(textureEncodeResponseMutex);
      if (textureEncodeResponseQueue.size() > 0) {
        entry = textureEncodeResponseQueue.front();
        textureEncodeResponseQueue.pop_front();
      } else {
        entry = nullptr;
      }
    }

    if (entry) {
      if (entry->type == TextureEncodeEntryType::MESH_BUFFER) {
        if (entry->resultSize > 0) {
          const std::string &id = entry->id;
          auto match = meshBuffers.find(id);
          if (match != meshBuffers.end()) {
            MeshBuffer &meshBuffer = match->second;

            if (meshBuffer.textureData) {
              SjpegFreeBuffer(meshBuffer.textureData);
            }
            meshBuffer.textureData = entry->result;
            meshBuffer.textureDataSize = entry->resultSize;

            pollMeshBufferIdList.push_back(id);
          }
        }
      } else {
        ML_LOG(Error, "%s: unknown texture encode entry type: %x", application_name, entry->type);
      }

      // cleanup
      glBindBuffer(GL_PIXEL_PACK_BUFFER, entry->pbo);
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      glDeleteBuffers(1, &entry->pbo);
      if (gl->HasBufferBinding(GL_PIXEL_PACK_BUFFER)) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->GetBufferBinding(GL_PIXEL_PACK_BUFFER));
      } else {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
      }
      delete entry;
    } else {
      break;
    }
  }

  if (pollMeshBufferIdList.size() > 0) {
    std::for_each(cameraMeshers.begin(), cameraMeshers.end(), [&](MLCameraMesher *m) {
      m->Poll(pollMeshBufferIdList);
    });
  }
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

// camera content shader
const char *cameraContentVsh = "\
#version 330\n\
\n\
in vec2 point;\n\
in vec2 uv;\n\
out vec2 vUvCamera;\n\
out vec2 vUvContent;\n\
\n\
float cameraAspectRatio = 1920.0/1080.0;\n\
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
const char *cameraContentFsh = "\
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

// camera raw shader
const char *cameraRawVsh = "\
#version 330\n\
\n\
in vec2 point;\n\
in vec2 uv;\n\
out vec2 vUvCamera;\n\
\n\
void main() {\n\
  vUvCamera = uv;\n\
  gl_Position = vec4(point, 1.0, 1.0);\n\
}\n\
";
const char *cameraRawFsh = "\
#version 330\n\
#extension GL_OES_EGL_image_external : enable\n\
\n\
in vec2 vUvCamera;\n\
out vec4 fragColor;\n\
\n\
uniform samplerExternalOES cameraInTexture;\n\
\n\
void main() {\n\
  fragColor = texture2D(cameraInTexture, vUvCamera);\n\
}\n\
";

// camera mesh shader
const char *cameraMeshVsh2 = "\
#version 330\n\
\n\
uniform mat4 projectionMatrix;\n\
uniform mat4 modelViewMatrix;\n\
\n\
in vec3 position;\n\
in vec2 uv;\n\
out vec2 vScreen;\n\
out vec3 vPosition;\n\
\n\
float cameraAspectRatio = 1920.0/1080.0;\n\
float renderAspectRatio = 1280.0/960.0;\n\
float xfactor = renderAspectRatio/cameraAspectRatio;\n\
float factor = 1.0/1.4;\n\
\n\
void main() {\n\
  vec4 modelViewPosition = modelViewMatrix * vec4(position, 1.0);\n\
  vec4 screenPosition = projectionMatrix * modelViewPosition;\n\
  vScreen = screenPosition.xy / screenPosition.w;\n\
  vScreen = vec2(vScreen.x * xfactor, vScreen.y) * factor;\n\
  vScreen = vScreen / 2.0 + 0.5;\n\
  vPosition = modelViewPosition.xyz;\n\
  vec2 uv2 = uv;\n\
  int gvid = gl_VertexID % 6;\n\
  if (gvid == 0) { // XXX can be an array\n\
    uv2.x -= 0.5/512.0;\n\
    uv2.y -= 0.5/512.0;\n\
  } else if (gvid == 1) {\n\
    uv2.x += 1.25/512.0;\n\
    uv2.y -= 0.5/512.0;\n\
  } else if (gvid == 2) {\n\
    uv2.x -= 0.5/512.0;\n\
    uv2.y += 1.25/512.0;\n\
  } else if (gvid == 3) {\n\
    uv2.x -= 1.25/512.0;\n\
    uv2.y += 0.5/512.0;\n\
  } else if (gvid == 4) {\n\
    uv2.x += 0.5/512.0;\n\
    uv2.y -= 1.25/512.0;\n\
  } else /* if (gvid == 5) */ {\n\
    uv2.x += 0.5/512.0;\n\
    uv2.y += 0.5/512.0;\n\
  }\n\
  gl_Position = vec4((uv2 - 0.5) * 2.0, screenPosition.z/screenPosition.w, 1.0);\n\
}\n\
";
const char *cameraMeshFsh2 = "\
#version 330\n\
\n\
uniform sampler2D cameraSnapshotTexture;\n\
\n\
in vec2 vScreen;\n\
in vec3 vPosition;\n\
out vec4 fragColor;\n\
\n\
void main() {\n\
  if (vScreen.x >= 0.0 && vScreen.x <= 1.0 && vScreen.y >= 0.0 && vScreen.y <= 1.0) {\n\
    vec3 unnormalizedNormal = cross(dFdx(vPosition), dFdy(vPosition));\n\
    if (unnormalizedNormal.z >= 0) {\n\
      fragColor = texture2D(cameraSnapshotTexture, vScreen);\n\
    } else {\n\
      discard;\n\
    }\n\
  } else {\n\
    discard;\n\
  }\n\
}\n\
";
NAN_METHOD(MLContext::InitLifecycle) {
  if (info[0]->IsFunction() && info[1]->IsFunction()) {
    eventsCb.Reset(Local<Function>::Cast(info[0]));
    keyboardEventsCb.Reset(Local<Function>::Cast(info[1]));

    uv_async_init(uv_default_loop(), &eventsAsync, RunEventsInMainThread);
    uv_async_init(uv_default_loop(), &keyboardEventsAsync, RunKeyboardEventsInMainThread);
    uv_async_init(uv_default_loop(), &cameraAsync, RunCameraInMainThread);
    // uv_async_init(uv_default_loop(), &cameraConvertAsync, RunCameraConvertInMainThread);
    uv_async_init(uv_default_loop(), &textureEncodeAsync, RunTextureEncodeInMainThread);
    // uv_sem_init(&cameraConvertSem, 0);
    uv_sem_init(&textureEncodeSem, 0);

    mlMesherConstructor.Reset(MLMesher::Initialize(Isolate::GetCurrent()));
    mlPlaneTrackerConstructor.Reset(MLPlaneTracker::Initialize(Isolate::GetCurrent()));
    mlHandTrackerConstructor.Reset(MLHandTracker::Initialize(Isolate::GetCurrent()));
    mlEyeTrackerConstructor.Reset(MLEyeTracker::Initialize(Isolate::GetCurrent()));
    mlCameraMesherConstructor.Reset(MLCameraMesher::Initialize(Isolate::GetCurrent()));

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

  // cameraa buffers
  GLuint pointBuffer;
  glGenBuffers(1, &pointBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
  static const GLfloat points[] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    -1.0f, 1.0f,
    1.0f, 1.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

  GLuint uvBuffer;
  glGenBuffers(1, &uvBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
  static const GLfloat uvs[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

  {
    // camera content shader
    glGenVertexArrays(1, &mlContext->cameraContentVao);
    glBindVertexArray(mlContext->cameraContentVao);

    // vertex Shader
    GLuint cameraContentVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cameraContentVertex, 1, &cameraContentVsh, NULL);
    glCompileShader(cameraContentVertex);
    GLint success;
    glGetShaderiv(cameraContentVertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraContentVertex, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera content vertex shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // fragment Shader
    GLuint cameraContentFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cameraContentFragment, 1, &cameraContentFsh, NULL);
    glCompileShader(cameraContentFragment);
    glGetShaderiv(cameraContentFragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraContentFragment, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera content fragment shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // shader Program
    mlContext->cameraContentProgram = glCreateProgram();
    glAttachShader(mlContext->cameraContentProgram, cameraContentVertex);
    glAttachShader(mlContext->cameraContentProgram, cameraContentFragment);
    glLinkProgram(mlContext->cameraContentProgram);
    glGetProgramiv(mlContext->cameraContentProgram, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(mlContext->cameraContentProgram, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera content program linking failed\n" << infoLog << std::endl;
      return;
    }

    mlContext->cameraContentPointLocation = glGetAttribLocation(mlContext->cameraContentProgram, "point");
    if (mlContext->cameraContentPointLocation == -1) {
      std::cout << "ML camera content program failed to get attrib location for 'point'" << std::endl;
      return;
    }
    mlContext->cameraContentUvLocation = glGetAttribLocation(mlContext->cameraContentProgram, "uv");
    if (mlContext->cameraContentUvLocation == -1) {
      std::cout << "ML camera content program failed to get attrib location for 'uv'" << std::endl;
      return;
    }
    mlContext->cameraContentCameraInTextureLocation = glGetUniformLocation(mlContext->cameraContentProgram, "cameraInTexture");
    if (mlContext->cameraContentCameraInTextureLocation == -1) {
      std::cout << "ML camera content program failed to get uniform location for 'cameraInTexture'" << std::endl;
      return;
    }
    mlContext->cameraContentContentTextureLocation = glGetUniformLocation(mlContext->cameraContentProgram, "contentTexture");
    if (mlContext->cameraContentContentTextureLocation == -1) {
      std::cout << "ML camera content program failed to get uniform location for 'contentTexture'" << std::endl;
      return;
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(cameraContentVertex);
    glDeleteShader(cameraContentFragment);

    // set up buffers
    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
    glEnableVertexAttribArray(mlContext->cameraContentPointLocation);
    glVertexAttribPointer(mlContext->cameraContentPointLocation, 2, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glEnableVertexAttribArray(mlContext->cameraContentUvLocation);
    glVertexAttribPointer(mlContext->cameraContentUvLocation, 2, GL_FLOAT, false, 0, 0);

    // glGenTextures(1, &mlContext->cameraContentCameraInTexture);
    mlContext->cameraContentContentTexture = colorTex;
    // glBindTexture(GL_TEXTURE_2D, mlContext->cameraContentCameraInTexture);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    /* glGenTextures(1, &mlContext->cameraContentCameraOutTexture);
    // glBindTexture(GL_TEXTURE_2D, mlContext->cameraContentCameraOutTexture);
    glBindTexture(GL_TEXTURE_2D, mlContext->cameraContentCameraOutTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, CAMERA_SIZE[0]/2, CAMERA_SIZE[1]/2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); */

    glGenFramebuffers(1, &mlContext->cameraContentDrawFbo);
    glGenFramebuffers(1, &mlContext->cameraContentReadFbo);
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->cameraContentDrawFbo);
    // glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mlContext->cameraContentCameraOutTexture, 0);

    {
      GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
      if (result != GL_FRAMEBUFFER_COMPLETE) {
        ML_LOG(Error, "%s: Failed to create camera content framebuffer: %x", application_name, result);
      }
    }
  }

  {
    // camera raw shader
    glGenVertexArrays(1, &mlContext->cameraRawVao);
    glBindVertexArray(mlContext->cameraRawVao);

    // vertex Shader
    GLuint cameraRawVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cameraRawVertex, 1, &cameraRawVsh, NULL);
    glCompileShader(cameraRawVertex);
    GLint success;
    glGetShaderiv(cameraRawVertex, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraRawVertex, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera raw vertex shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // fragment Shader
    GLuint cameraRawFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cameraRawFragment, 1, &cameraRawFsh, NULL);
    glCompileShader(cameraRawFragment);
    glGetShaderiv(cameraRawFragment, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(cameraRawFragment, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera raw fragment shader compilation failed:\n" << infoLog << std::endl;
      return;
    };

    // shader Program
    mlContext->cameraRawProgram = glCreateProgram();
    glAttachShader(mlContext->cameraRawProgram, cameraRawVertex);
    glAttachShader(mlContext->cameraRawProgram, cameraRawFragment);
    glLinkProgram(mlContext->cameraRawProgram);
    glGetProgramiv(mlContext->cameraRawProgram, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[4096];
      GLsizei length;
      glGetShaderInfoLog(mlContext->cameraRawProgram, sizeof(infoLog), &length, infoLog);
      infoLog[length] = '\0';
      std::cout << "ML camera raw program linking failed\n" << infoLog << std::endl;
      return;
    }

    mlContext->cameraRawPointLocation = glGetAttribLocation(mlContext->cameraRawProgram, "point");
    if (mlContext->cameraRawPointLocation == -1) {
      std::cout << "ML camera raw program failed to get attrib location for 'point'" << std::endl;
      return;
    }
    mlContext->cameraRawUvLocation = glGetAttribLocation(mlContext->cameraRawProgram, "uv");
    if (mlContext->cameraRawUvLocation == -1) {
      std::cout << "ML camera raw program failed to get attrib location for 'uv'" << std::endl;
      return;
    }
    mlContext->cameraRawCameraInTextureLocation = glGetUniformLocation(mlContext->cameraRawProgram, "cameraInTexture");
    if (mlContext->cameraRawCameraInTextureLocation == -1) {
      std::cout << "ML camera raw program failed to get uniform location for 'cameraInTexture'" << std::endl;
      return;
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(cameraRawVertex);
    glDeleteShader(cameraRawFragment);

    glGenTextures(1, &mlContext->cameraRawCameraInTexture);

    // set up buffers
    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
    glEnableVertexAttribArray(mlContext->cameraRawPointLocation);
    glVertexAttribPointer(mlContext->cameraRawPointLocation, 2, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glEnableVertexAttribArray(mlContext->cameraRawUvLocation);
    glVertexAttribPointer(mlContext->cameraRawUvLocation, 2, GL_FLOAT, false, 0, 0);

    glGenFramebuffers(1, &mlContext->cameraRawFbo);

    {
      GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
      if (result != GL_FRAMEBUFFER_COMPLETE) {
        ML_LOG(Error, "%s: Failed to create camera framebuffer: %x", application_name, result);
      }
    }
  }

  {
    // camera mesh shader 2
    glGenVertexArrays(1, &mlContext->cameraMeshVao2);

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
    mlContext->cameraMeshCameraSnapshotTextureLocation = glGetUniformLocation(mlContext->cameraMeshProgram2, "cameraSnapshotTexture");
    if (mlContext->cameraMeshCameraSnapshotTextureLocation == -1) {
      std::cout << "ML camera mesh 2 program failed to get uniform location for 'cameraSnapshotTexture'" << std::endl;
      return;
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(cameraMeshVertex);
    glDeleteShader(cameraMeshFragment);
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

    uv_sem_post(&textureEncodeSem);
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

        {
          const MLGraphicsVirtualCameraInfo &cameraInfo = mlContext->virtual_camera_array.virtual_cameras[0];
          const MLTransform &transform = cameraInfo.transform;
          const MLVec3f &position = addVectors(transform.position, applyVectorQuaternion(MLVec3f{0.0f, 0.0f, -1.0f}, transform.rotation));
          const milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
          cameraPositions.push_back(CameraPosition{position, now});

          while (cameraPositions.size() > 0) {
            const CameraPosition &cameraPosition = cameraPositions.front();
            if (cameraPosition.ms < (now - CAMERA_ADJUST_DELAY)) {
              cameraPositions.pop_front();
            } else {
              break;
            }
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

NAN_METHOD(MLContext::RequestCameraMeshing) {
  if (!cameraConnected) {
    MLResult result = connectCamera();
    if (result != MLResult_Ok) {
      ML_LOG(Error, "%s: Failed to connect camera meshing: %x", application_name, result);
      Nan::ThrowError("failed to connect camera meshing");
      return;
    }
  }

  Local<Function> mlCameraMesherCons = Nan::New(mlCameraMesherConstructor);
  Local<Object> mlCameraMesherObj = mlCameraMesherCons->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), 0, nullptr).ToLocalChecked();
  info.GetReturnValue().Set(mlCameraMesherObj);

  cameraMeshEnabled = true;
}

NAN_METHOD(MLContext::RequestCameraStream) {
  if (info[0]->IsString()) {
    if (!cameraConnected) {
      MLResult result = connectCamera();
      if (result != MLResult_Ok) {
        ML_LOG(Error, "%s: Failed to connect camera stream: %x", application_name, result);
        Nan::ThrowError("failed to connect camera stream");
        return;
      }
    }

    Local<String> url = Local<String>::Cast(info[0]);
    String::Utf8Value urlValue(url);
    {
      // std::unique_lock<std::mutex> lock(cameraRequestsMutex);

      MLStream *stream = new MLStream(
        0,
        20,
        CAMERA_STREAM_SIZE[0],
        CAMERA_STREAM_SIZE[1],
        3500 * 1024,
        // 1000 * 1024,
        std::string("main"),
        // std::string("high444"),
        std::string(*urlValue, urlValue.length())
      );
      stream->start();
      cameraStreams.push_back(stream);
    }
  } else {
    Nan::ThrowError("invalid arguments");
    return;
  }
}

NAN_METHOD(MLContext::RequestDepthPopulation) {
  if (info[0]->IsBoolean()) {
    depthEnabled = info[0]->BooleanValue();
  } else {
    Nan::ThrowError("invalid arguments");
  }
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

void getUvs(MLVec3f *vertex, uint32_t vertex_count, uint16_t *index, uint16_t index_count, std::vector<MLVec3f> *vertices, std::vector<Uv> *uvs) {
  if (index_count > 0) {
    vertices->clear();
    vertices->resize(index_count);
    for (uint16_t i = 0; i < index_count; i++) {
      (*vertices)[i] = vertex[index[i]];
    }

    uvs->clear();
    uvs->resize(vertices->size());

    const size_t width = (size_t)sqrt((float)vertices->size()/4.0f);
    const size_t &height = width;
    size_t x = 0;
    size_t y = 0;
    for (size_t i = 0; i < vertices->size()/3; i++) {
      if ((i % 2) == 0) {
        (*uvs)[i*3] = {
          ((float)x/(float)width*(float)(MESH_TEXTURE_SIZE[0])+0.5f)/(float)(MESH_TEXTURE_SIZE[0]),
          ((float)y/(float)height*(float)(MESH_TEXTURE_SIZE[1])+0.5f)/(float)(MESH_TEXTURE_SIZE[1])
        };
        (*uvs)[i*3+1] = {
          ((float)(x+1)/(float)width*(float)(MESH_TEXTURE_SIZE[0])-1.25f)/(float)(MESH_TEXTURE_SIZE[0]),
          ((float)y/(float)height*(float)(MESH_TEXTURE_SIZE[1])+0.5f)/(float)(MESH_TEXTURE_SIZE[1])
        };
        (*uvs)[i*3+2] = {
          ((float)x/(float)width*(float)(MESH_TEXTURE_SIZE[0])+0.5f)/(float)(MESH_TEXTURE_SIZE[0]),
          ((float)(y+1)/(float)height*(float)(MESH_TEXTURE_SIZE[1])-1.25f)/(float)(MESH_TEXTURE_SIZE[1])
        };
      } else {
        (*uvs)[i*3] = {
          ((float)x/(float)width*(float)(MESH_TEXTURE_SIZE[0])+1.25f)/(float)(MESH_TEXTURE_SIZE[0]),
          ((float)(y+1)/(float)height*(float)(MESH_TEXTURE_SIZE[1])-0.5f)/(float)(MESH_TEXTURE_SIZE[1])
        };
        (*uvs)[i*3+1] = {
          ((float)(x+1)/(float)width*(float)(MESH_TEXTURE_SIZE[0])-0.5f)/(float)(MESH_TEXTURE_SIZE[0]),
          ((float)y/(float)height*(float)(MESH_TEXTURE_SIZE[1])+1.25f)/(float)(MESH_TEXTURE_SIZE[1])
        };
        (*uvs)[i*3+2] = {
          ((float)(x+1)/(float)width*(float)(MESH_TEXTURE_SIZE[0])-0.5f)/(float)(MESH_TEXTURE_SIZE[0]),
          ((float)(y+1)/(float)height*(float)(MESH_TEXTURE_SIZE[1])-0.5f)/(float)(MESH_TEXTURE_SIZE[1])
        };

        x++;
        if ((x + 1) >= width) {
          x = 0;
          y++;
        }
      }
    }
  } else {
    vertices->clear();
    uvs->clear();
  }
}

bool frustumCheck(const CameraMeshPreviewRequest &cameraMeshPreviewRequest, const MeshBuffer &meshBuffer) {
  return vectorLengthSq(subVectors(cameraMeshPreviewRequest.position, meshBuffer.center)) <= (2.0f*2.0f) && frustumIntersectsSphere(cameraMeshPreviewRequest.frustum, MLSpheref{meshBuffer.center, 1});
}
}

void renderCameras(const std::vector<std::string> &meshBufferIdRenderList, const std::vector<CameraMeshPreviewRequest *> &cameraMeshPreviewRenderList) {
  bool rendering = false;
  if (meshBufferIdRenderList.size() > 0) {
    for (auto iter = meshBufferIdRenderList.begin(); iter != meshBufferIdRenderList.end(); iter++) {
      const std::string &id = *iter;
      MeshBuffer &meshBuffer = meshBuffers[id];

      bool localRendering = false;

      for (auto iter2 = cameraMeshPreviewRequests.begin(); iter2 != cameraMeshPreviewRequests.end(); iter2++) {
        CameraMeshPreviewRequest &cameraMeshPreviewRequest = *iter2;

        if (cameraMeshPreviewRequest.texture && frustumCheck(cameraMeshPreviewRequest, meshBuffer)) {
          if (!rendering) {
            MeshBuffer::beginRenderCameraAll();
            rendering = true;
          }
          if (!localRendering) {
            meshBuffer.beginRenderCamera();
            localRendering = true;
          }
          meshBuffer.renderCamera(cameraMeshPreviewRequest);
        }
      }
    }
  }
  if (cameraMeshPreviewRenderList.size() > 0) {
    for (auto iter = meshBuffers.begin(); iter != meshBuffers.end(); iter++) {
      const std::string &id = iter->first;
      MeshBuffer &meshBuffer = iter->second;

      auto match = std::find(meshBufferIdRenderList.begin(), meshBufferIdRenderList.end(), id);
      if (match == meshBufferIdRenderList.end()) {
        bool localRendering = false;

        for (auto iter2 = cameraMeshPreviewRenderList.begin(); iter2 != cameraMeshPreviewRenderList.end(); iter2++) {
          CameraMeshPreviewRequest &cameraMeshPreviewRequest = **iter2;

          if (cameraMeshPreviewRequest.texture != 0 && frustumCheck(cameraMeshPreviewRequest, meshBuffer)) {
            if (!rendering) {
              MeshBuffer::beginRenderCameraAll();
              rendering = true;
            }
            if (!localRendering) {
              meshBuffer.beginRenderCamera();
              localRendering = true;
            }
            meshBuffer.renderCamera(cameraMeshPreviewRequest);
          }
        }
      }
    }
  }

  if (rendering) {
    MeshBuffer::endRenderCamera();
  }
}

NAN_METHOD(MLContext::PostPollEvents) {
  MLContext *mlContext = ObjectWrap::Unwrap<MLContext>(Local<Object>::Cast(info[0]));
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[1]));
  GLuint fbo = info[2]->Uint32Value();
  NATIVEwindow *window = application_context.window;

  std::vector<std::string> meshBufferIdRenderList;
  std::vector<CameraMeshPreviewRequest *> cameraMeshPreviewRenderList;

  if (cameraMeshEnabled) {
    // read pixel buffers
    /* if (textureEncodePbos.size() > 0) {
      size_t textureEncodeRequestQueueSize;
      {
        std::lock_guard<mutex> lock(textureEncodeRequestMutex);
        textureEncodeRequestQueueSize = textureEncodeRequestQueue.size();
      }
      while (textureEncodePbos.size() > 0 && textureEncodeRequestQueueSize < MAX_TEXTURE_QUEUE_SIZE) {
        const std::pair<std::string, GLuint> &iter = textureEncodePbos.front();
        const std::string &id = iter.first;
        GLuint pbo = iter.second;

        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        uint8_t *textureDataRgb = (uint8_t *)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, MESH_TEXTURE_SIZE[0] * MESH_TEXTURE_SIZE[1] * 4, GL_MAP_READ_BIT);

        TextureEncodeRequestEntry *requestEntry = new TextureEncodeRequestEntry(TextureEncodeEntryType::MESH_BUFFER, id, MESH_TEXTURE_SIZE[0], MESH_TEXTURE_SIZE[1], pbo, textureDataRgb);
        {
          std::lock_guard<mutex> lock(textureEncodeRequestMutex);
          textureEncodeRequestQueue.push_back(requestEntry);
          textureEncodeRequestQueueSize = textureEncodeRequestQueue.size();
        }
        uv_sem_post(&textureEncodeSem);

        textureEncodePbos.pop_front();
      }

      if (gl->HasBufferBinding(GL_PIXEL_PACK_BUFFER)) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->GetBufferBinding(GL_PIXEL_PACK_BUFFER));
      } else {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
      }
    } */
    tickGetPixelsResponse();

    // queue pixel buffers
    {
      for (auto iter = meshBuffers.begin(); iter != meshBuffers.end(); iter++) {
        const std::string &id = iter->first;
        MeshBuffer &meshBuffer = iter->second;

        if (meshBuffer.textureDirty) {
          meshBuffer.readPixels(id, [id](GLuint pbo, uint8_t *textureDataRgb) -> void {
            TextureEncodeRequestEntry *requestEntry = new TextureEncodeRequestEntry(TextureEncodeEntryType::MESH_BUFFER, id, MESH_TEXTURE_SIZE[0], MESH_TEXTURE_SIZE[1], pbo, textureDataRgb);
            {
              std::lock_guard<mutex> lock(textureEncodeRequestMutex);
              textureEncodeRequestQueue.push_back(requestEntry);
            }

            uv_sem_post(&textureEncodeSem);
          });
        }
      }
    }

    // remove overflowed camera mesh preview requests
    std::vector<std::deque<CameraMeshPreviewRequest>::iterator> eraseList;
    {
      size_t numCameraMeshTextures = 0;
      for (auto iter = cameraMeshPreviewRequests.begin(); iter != cameraMeshPreviewRequests.end(); iter++) {
        CameraMeshPreviewRequest &cameraMeshPreviewRequest = *iter;
        if (cameraMeshPreviewRequest.texture != 0) {
          numCameraMeshTextures++;
        }
      }
      if (numCameraMeshTextures > MAX_CAMERA_MESH_TEXTURES) {
        for (auto iter = cameraMeshPreviewRequests.begin(); iter != cameraMeshPreviewRequests.end(); iter++) {
          CameraMeshPreviewRequest &cameraMeshPreviewRequest = *iter;

          if (cameraMeshPreviewRequest.texture != 0) {
            glDeleteTextures(1, &cameraMeshPreviewRequest.texture);
            eraseList.push_back(iter);

            numCameraMeshTextures--;
            if (numCameraMeshTextures <= MAX_CAMERA_MESH_TEXTURES) {
              break;
            }
          }
        }
      }
    }
    for (auto iterIter = eraseList.begin(); iterIter != eraseList.end(); iterIter++) {
      auto iter = *iterIter;
      cameraMeshPreviewRequests.erase(iter);
    }

    // camera stream read
    {
      for (auto iter = cameraStreamRequests.begin(); iter != cameraStreamRequests.end(); iter++) {
        CameraStreamRequest *cameraStreamRequest = *iter;
        GLuint texture = cameraStreamRequest->texture;

        getPixels(std::string(), CAMERA_STREAM_SIZE[0], CAMERA_STREAM_SIZE[1], texture, false, [texture](GLuint pbo, uint8_t *framebufferDataRgb) -> void {
          for (auto iter = cameraStreams.begin(); iter != cameraStreams.end(); iter++) {
            MLStream &stream = **iter;
            stream.pushFramebuffer(pbo, framebufferDataRgb);
          }

          glDeleteTextures(1, &texture); // XXX handle cleanup case
        });

        /* glBindFramebuffer(GL_READ_FRAMEBUFFER, mlContext->cameraContentReadFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        GLuint pbo;
        glGenBuffers(1, &pbo);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        glBufferData(GL_PIXEL_PACK_BUFFER, CAMERA_STREAM_SIZE[0] * CAMERA_STREAM_SIZE[1] * 4, 0, GL_STREAM_READ);
        glReadPixels(0, 0, CAMERA_STREAM_SIZE[0], CAMERA_STREAM_SIZE[1], GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL); */

        delete cameraStreamRequest;

        /* if (gl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
          glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
        } else {
          glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->defaultFramebuffer);
        }
        if (gl->HasBufferBinding(GL_PIXEL_PACK_BUFFER)) {
          glBindBuffer(GL_PIXEL_PACK_BUFFER, gl->GetBufferBinding(GL_PIXEL_PACK_BUFFER));
        } else {
          glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        } */
      }
      cameraStreamRequests.clear();
    }

    // capture camera
    const milliseconds now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    for (auto iter = cameraMeshPreviewRequests.begin(); iter != cameraMeshPreviewRequests.end(); iter++) {
      CameraMeshPreviewRequest &cameraMeshPreviewRequest = *iter;

      if (cameraMeshPreviewRequest.texture == 0 && now >= cameraMeshPreviewRequest.ms) {
        bool isStable = false;
        {
          float stability = 0;
          bool first = true;
          MLVec3f lastPosition;
          for (auto iter : cameraPositions) {
            if (iter.ms >= (cameraMeshPreviewRequest.ms - CAMERA_ADJUST_DELAY) && iter.ms < cameraMeshPreviewRequest.ms) {
              if (first) {
                first = false;
              } else {
                stability += vectorLength(subVectors(iter.position, lastPosition));
              }
              lastPosition = iter.position;
            }
          }
          isStable = stability > 0.0f && stability < 0.03f;
        }
        bool isCameraStreamEnabled = cameraStreams.size() > 0;
        if (isStable || isCameraStreamEnabled) {
          MLHandle output;
          MLResult result = MLCameraGetPreviewStream(&output);
          if (result == MLResult_Ok) {
            // ANativeWindowBuffer_t *aNativeWindowBuffer = (ANativeWindowBuffer_t *)output;
            EGLImageKHR yuv_img = eglCreateImageKHR(window->display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, (EGLClientBuffer)(void*)output, nullptr);
            bool cameraRawInTextureBound = false;

            // camera mesh
            if (isStable) {
              std::deque<CameraMeshPreviewRequest>::iterator match = cameraMeshPreviewRequests.end();
              for (auto iter2 = cameraMeshPreviewRequests.begin(); iter2 != cameraMeshPreviewRequests.end(); iter2++) {
                CameraMeshPreviewRequest &cameraMeshPreviewRequest2 = *iter2;
                if (
                  cameraMeshPreviewRequest2.texture != 0 &&
                  vectorLength(subVectors(cameraMeshPreviewRequest.position, cameraMeshPreviewRequest2.position)) < 0.03f &&
                  getAngleBetweenQuaternions(cameraMeshPreviewRequest.rotation, cameraMeshPreviewRequest2.rotation) < (float)M_PI/16.0f
                ) {
                  match = iter2;
                  break;
                }
              }

              glBindVertexArray(mlContext->cameraRawVao);
              glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->cameraRawFbo);
              glUseProgram(mlContext->cameraRawProgram);

              glActiveTexture(GL_TEXTURE0);
              if (match != cameraMeshPreviewRequests.end()) {
                // cameraMeshPreviewRequest.position = match->position;
                // cameraMeshPreviewRequest.rotation = match->rotation;
                // cameraMeshPreviewRequest.modelViewInverse = match->modelViewInverse;
                // cameraMeshPreviewRequest.projection = match->projection;
                // cameraMeshPreviewRequest.frustum = match->frustum;
                cameraMeshPreviewRequest.texture = match->texture;
                // cameraMeshPreviewRequest.ms = match->ms;
                match->texture = 0;

                glBindTexture(GL_TEXTURE_2D, cameraMeshPreviewRequest.texture);

                eraseList.push_back(match);
              } else {
                glGenTextures(1, &cameraMeshPreviewRequest.texture);
                glBindTexture(GL_TEXTURE_2D, cameraMeshPreviewRequest.texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, CAMERA_SIZE[0], CAMERA_SIZE[1], 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
              }
              glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cameraMeshPreviewRequest.texture, 0);

              if (!cameraRawInTextureBound) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, mlContext->cameraRawCameraInTexture);
                glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, yuv_img);
                cameraRawInTextureBound = true;
              }
              glUniform1i(mlContext->cameraRawCameraInTextureLocation, 1);

              glViewport(0, 0, CAMERA_SIZE[0], CAMERA_SIZE[1]);
              glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

              cameraMeshPreviewRenderList.push_back(&cameraMeshPreviewRequest);

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
              if (gl->HasTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D)) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D));
              } else {
                glBindTexture(GL_TEXTURE_2D, 0);
              }
              if (gl->HasTextureBinding(GL_TEXTURE1, GL_TEXTURE_EXTERNAL_OES)) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, gl->GetTextureBinding(GL_TEXTURE1, GL_TEXTURE_EXTERNAL_OES));
              } else {
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
              }
              glActiveTexture(gl->activeTexture);
            }

            // camera stream
            if (isCameraStreamEnabled) {
              glBindVertexArray(mlContext->cameraContentVao);

              glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mlContext->cameraContentDrawFbo);
              GLuint texture;
              glGenTextures(1, &texture);
              glActiveTexture(GL_TEXTURE0);
              glBindTexture(GL_TEXTURE_2D, texture);
              glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, CAMERA_STREAM_SIZE[0], CAMERA_STREAM_SIZE[1], 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
              glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

              glUseProgram(mlContext->cameraContentProgram);

              glBindTexture(GL_TEXTURE_2D, mlContext->cameraContentContentTexture);
              glUniform1i(mlContext->cameraContentContentTextureLocation, 0);

              if (!cameraRawInTextureBound) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, mlContext->cameraRawCameraInTexture);
                glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, yuv_img);
                cameraRawInTextureBound = true;
              }
              glUniform1i(mlContext->cameraContentCameraInTextureLocation, 1);

              glViewport(0, 0, CAMERA_STREAM_SIZE[0], CAMERA_STREAM_SIZE[1]);
              glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

              CameraStreamRequest *cameraStreamRequest = new CameraStreamRequest{texture};
              cameraStreamRequests.push_back(cameraStreamRequest);

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
              if (gl->HasTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D)) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(GL_TEXTURE0, GL_TEXTURE_2D));
              } else {
                glBindTexture(GL_TEXTURE_2D, 0);
              }
              if (gl->HasTextureBinding(GL_TEXTURE1, GL_TEXTURE_EXTERNAL_OES)) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, gl->GetTextureBinding(GL_TEXTURE1, GL_TEXTURE_EXTERNAL_OES));
              } else {
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
              }
              glActiveTexture(gl->activeTexture);
            }

            eglDestroyImageKHR(window->display, yuv_img);
          } else {
            ML_LOG(Error, "%s: failed to get camera preview stream %x", application_name, result);

            eraseList.push_back(iter);
          }
        }
        if (!isStable) {
          eraseList.push_back(iter);
        }
      }
    }
    // erase queued camera mesh preview requests
    for (auto iterIter = eraseList.begin(); iterIter != eraseList.end(); iterIter++) {
      auto iter = *iterIter;
      cameraMeshPreviewRequests.erase(iter);
    }
  }

  if (meshInfoRequestPending) {
    MLResult result = MLMeshingGetMeshInfoResult(meshTracker, meshInfoRequestHandle, &meshInfo);
    if (result == MLResult_Ok) {
      uint32_t dataCount = meshInfo.data_count;

      meshRequestSpecMap.clear();
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
        meshRequestSpecMap[id] = {
          extents.center,
          state == MLMeshingMeshState_New,
          state == MLMeshingMeshState_Deleted,
          state == MLMeshingMeshState_Unchanged
        };
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
  bool rendered = false;
  if (meshRequestsPending && meshRequestPending) {
    MLResult result = MLMeshingGetMeshResult(meshTracker, meshRequestHandle, &mesh);
    if (result == MLResult_Ok) {
      MLMeshingBlockMesh *blockMeshes = mesh.data;
      uint32_t dataCount = mesh.data_count;

      for (uint32_t i = 0; i < dataCount; i++) {
        MLMeshingBlockMesh &blockMesh = blockMeshes[i];
        const std::string &id = id2String(blockMesh.id);

        const MeshRequestSpec &meshRequestSpec = meshRequestSpecMap[id];
        if (!meshRequestSpec.isRemoved) {
          const bool rerenderBlock = cameraMeshEnabled && !meshRequestSpec.isUnchanged;

          MeshBuffer *meshBuffer;
          auto iter = meshBuffers.find(id);
          if (iter != meshBuffers.end()) {
            meshBuffer = &iter->second;

            if (rerenderBlock) {
              glBindFramebuffer(GL_DRAW_FRAMEBUFFER, meshBuffer->fbo);

              glClearColor(0.1, 0.1, 0.1, 1.0);
              glClear(GL_COLOR_BUFFER_BIT);
              glClearColor(0.0, 0.0, 0.0, 1.0);
            }
          } else {
            GLuint buffers[5];
            glGenBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
            GLuint fbo;
            glGenFramebuffers(1, &fbo);
            GLuint texture;
            glGenTextures(1, &texture);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, MESH_TEXTURE_SIZE[0], MESH_TEXTURE_SIZE[1], 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

            glClearColor(0.1, 0.1, 0.1, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(0.0, 0.0, 0.0, 1.0);

            meshBuffers[id] = MeshBuffer{
              buffers[0],
              buffers[1],
              buffers[2],
              buffers[3],
              buffers[4],
              fbo,
              texture,
              nullptr,
              0,
              nullptr,
              nullptr,
              0,
              std::vector<MLVec3f>(),
              std::vector<Uv>(),
              nullptr,
              0,
              MLVec3f{0, 0, 0},
              true,
              false,
              false,
            };
            meshBuffer = &meshBuffers[id];
          }

          std::vector<MLVec3f> localVertices;
          std::vector<Uv> localUvs;
          if (rerenderBlock) {
            getUvs(blockMesh.vertex, blockMesh.vertex_count, blockMesh.index, blockMesh.index_count, &localVertices, &localUvs);
          }

          meshBuffer->setBuffers(id, (float *)(&blockMesh.vertex->values), blockMesh.vertex_count * 3, (float *)(&blockMesh.normal->values), blockMesh.index, blockMesh.index_count, &localVertices, &localUvs, meshRequestSpec.center, meshRequestSpec.isNew, meshRequestSpec.isUnchanged);

          if (rerenderBlock) {
            meshBufferIdRenderList.push_back(id);
          }
        } else {
          auto iter = meshBuffers.find(id);
          if (iter != meshBuffers.end()) {
            MeshBuffer *meshBuffer = &iter->second;
            GLuint buffers[] = {
              meshBuffer->positionBuffer,
              meshBuffer->normalBuffer,
              meshBuffer->vertexBuffer,
              meshBuffer->uvBuffer,
              meshBuffer->indexBuffer,
            };
            glDeleteBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
            glDeleteFramebuffers(1, &meshBuffer->fbo);
            glDeleteTextures(1, &meshBuffer->texture);
            if (meshBuffer->textureData) {
              SjpegFreeBuffer(meshBuffer->textureData);
            }
            meshBuffers.erase(iter);
          }
        }
      }

      renderCameras(meshBufferIdRenderList, cameraMeshPreviewRenderList);
      rendered = true;

      std::for_each(meshers.begin(), meshers.end(), [&](MLMesher *m) {
        m->Poll();
      });
      std::for_each(cameraMeshers.begin(), cameraMeshers.end(), [&](MLCameraMesher *m) {
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
  if (!rendered) {
    renderCameras(meshBufferIdRenderList, cameraMeshPreviewRenderList);
  }

  /* for (auto iter = cameraStreams.begin(); iter != cameraStreams.end(); iter++) {
    MLStream &stream = **iter;
    stream.tick();
  } */

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

  ml::MLStream::Init();

  return scope.Escape(ml::MLContext::Initialize(Isolate::GetCurrent()));
}

#endif

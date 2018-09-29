/*
 * webgl.h
 *
 *  Created on: Dec 13, 2011
 *      Author: ngk437
 */

#ifndef _MAGICLEAP_H_
#define _MAGICLEAP_H_

#include <v8.h>
#include <node.h>
#include <nan.h>
#include <defines.h>
#include <dlfcn.h>
#include <cmath>
#include <vector>
#include <map>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <chrono>
#include <webgl.h>
#include <egl/include/egl.h>
#include <ml_graphics.h>
#include <ml_head_tracking.h>
#include <ml_hand_tracking.h>
#include <ml_perception.h>
#include <ml_snapshot.h>
#include <ml_meshing2.h>
#include <ml_planes.h>
#include <ml_camera.h>
#include <ml_eye_tracking.h>
#include <ml_privilege_ids.h>
#include <ml_privilege_functions.h>
#include <ml_input.h>
#include <ml_gesture.h>
#include <ml_lifecycle.h>
#include <ml_logging.h>

#include "ml-window.h"
#include "sjpeg.h"

#define M_PI 3.14159265358979323846

using namespace v8;
using namespace node;
using namespace std::chrono;
using namespace std::chrono_literals;

#define MAX_NUM_PLANES (32)
#define PLANE_ENTRY_SIZE (3 + 4 + 2 + 1)
#define CONTROLLER_ENTRY_SIZE (3 + 4 + 6)
#define CAMERA_REQUEST_PLANE_BUFFER_SIZE (5 * 1024 * 1024)
constexpr milliseconds CAMERA_PREVIEW_DELAY = 150ms;
constexpr milliseconds CAMERA_ADJUST_DELAY = CAMERA_PREVIEW_DELAY;
constexpr size_t MAX_CAMERA_MESH_TEXTURES = 10;

namespace ml {

// enums

enum DummyValue {
  STOPPED,
  RUNNING,
  PAUSED,
};

enum Event {
  NEW_INIT_ARG,
  STOP,
  PAUSE,
  RESUME,
  UNLOAD_RESOURCES,
};

enum KeyboardEventType {
  CHAR,
  KEY_DOWN,
  KEY_UP,
};

// classes

class MLContext;

class MLSpheref {
public:
  MLVec3f center;
  float radius;
};

class MLPlanef {
public:
  MLVec3f normal;
  float constant;
};

class MLFrustumf {
public:
  MLPlanef planes[6];
};

struct application_context_t {
  int dummy_value;
  MLContext *mlContext;
  WebGLRenderingContext *gl;
  NATIVEwindow *window;
};

class KeyboardEvent {
public:
  KeyboardEvent(KeyboardEventType type, uint32_t char_utf32);
  KeyboardEvent(KeyboardEventType type, MLKeyCode key_code, uint32_t modifier_mask);

  KeyboardEventType type;
  uint32_t char_utf32;
  MLKeyCode key_code;
  uint32_t modifier_mask;
};

class Uv {
public:
  float u;
  float v;
};
class CameraMeshPreviewRequest {
public:
  MLVec3f position;
  MLQuaternionf rotation;
  MLMat4f modelViewInverse;
  MLMat4f projection;
  MLFrustumf frustum;
  GLuint texture;
  milliseconds ms;
};

class MeshBuffer {
public:
  // MeshBuffer(GLuint positionBuffer, GLuint normalBuffer, GLuint vertexBuffer, GLuint uvBuffer, GLuint indexBuffer, GLuint fbo, GLuint texture);
  // MeshBuffer(const MeshBuffer &meshBuffer);
  // MeshBuffer();
  void setBuffers(float *positions, uint32_t numPositions, float *normals, uint16_t *indices, uint16_t numIndices, std::vector<MLVec3f> *vertices, std::vector<Uv> *uvs, bool isNew, bool isUnchanged, bool textureDirty);
  static void beginRenderCameraAll();
  void beginRenderCamera();
  void renderCamera(const CameraMeshPreviewRequest &cameraMeshPreviewRequest);
  static void endRenderCamera();

  GLuint positionBuffer;
  GLuint normalBuffer;
  GLuint vertexBuffer;
  GLuint uvBuffer;
  GLuint indexBuffer;
  GLuint fbo;
  GLuint texture;
  float *positions;
  uint32_t numPositions;
  float *normals;
  uint16_t *indices;
  uint16_t numIndices;
  std::vector<MLVec3f> vertices;
  std::vector<Uv> uvs;
  std::vector<uint8_t> textureData;
  bool isNew;
  bool isUnchanged;
  bool textureDirty;
};

enum TextureEncodeEntryType {
  MESH_BUFFER,
};

class TextureEncodeRequestEntry {
public:
  TextureEncodeRequestEntry(TextureEncodeEntryType type, const std::string id, int width, int height, uint8_t *data);

  TextureEncodeEntryType type;
  std::string id;
  int width;
  int height;
  uint8_t *data;
};

class TextureEncodeResponseEntry {
public:
  TextureEncodeResponseEntry(TextureEncodeEntryType type, const std::string id, uint8_t *result, size_t resultSize);

  TextureEncodeEntryType type;
  std::string id;
  uint8_t *result;
  size_t resultSize;
};

class MLMesher : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLMesher();
  ~MLMesher();

  static NAN_METHOD(New);
  static NAN_GETTER(OnMeshGetter);
  static NAN_SETTER(OnMeshSetter);
  static NAN_METHOD(Destroy);

  void Poll();
  // void Repoll();

// protected:
  Nan::Persistent<Function> cb;
};

class MLPlaneTracker : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLPlaneTracker();
  ~MLPlaneTracker();

  static NAN_METHOD(New);
  static NAN_GETTER(OnPlanesGetter);
  static NAN_SETTER(OnPlanesSetter);
  static NAN_METHOD(Destroy);

  void Poll();

// protected:
  Nan::Persistent<Function> cb;
};

class MLHandTracker : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLHandTracker();
  ~MLHandTracker();

  static NAN_METHOD(New);
  static NAN_GETTER(OnHandsGetter);
  static NAN_SETTER(OnHandsSetter);
  static NAN_GETTER(OnGestureGetter);
  static NAN_SETTER(OnGestureSetter);
  static NAN_METHOD(Destroy);

  void Poll();

// protected:
  Nan::Persistent<Function> cb;
  Nan::Persistent<Function> ongesture;
};

class MLEyeTracker : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLEyeTracker();
  ~MLEyeTracker();

  static NAN_METHOD(New);
  static NAN_GETTER(FixationGetter);
  static NAN_GETTER(EyesGetter);
  static NAN_METHOD(Destroy);

  void Poll(MLSnapshot *snapshot);

// protected:
  MLTransform transform;
  MLTransform leftTransform;
  bool leftBlink;
  MLTransform rightTransform;
  bool rightBlink;
};

class MLCameraMesher : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLCameraMesher();
  ~MLCameraMesher();

  static NAN_METHOD(New);
  static NAN_GETTER(OnMeshGetter);
  static NAN_SETTER(OnMeshSetter);
  static NAN_METHOD(Destroy);

  void Poll();
  void Poll(const std::vector<std::string> &meshBufferIds);
  // void Repoll();

// protected:
  Nan::Persistent<Function> cb;
};

class CameraRequest {
public:
  CameraRequest(Local<Function> cbFn);
  void Set(int width, int height, uint8_t *data, size_t size);
  void Poll();

// protected:
  Nan::Persistent<Function> cbFn;
  int width;
  int height;
  Nan::Persistent<ArrayBuffer> data;
};

class MeshRequestSpec {
public:
  MLVec3f center;
  bool isNew;
  bool isRemoved;
  bool isUnchanged;
};

class CameraPosition {
public:
  MLVec3f position;
  milliseconds ms;
};

class MLContext : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

// protected:
  MLContext();
  ~MLContext();

  static NAN_METHOD(New);
  static NAN_METHOD(InitLifecycle);
  static NAN_METHOD(DeinitLifecycle);
  static NAN_METHOD(Present);
  static NAN_METHOD(Exit);
  static NAN_METHOD(WaitGetPoses);
  static NAN_METHOD(SubmitFrame);
  static NAN_METHOD(IsPresent);
  static NAN_METHOD(IsSimulated);
  static NAN_METHOD(OnPresentChange);
  static NAN_METHOD(RequestMeshing);
  static NAN_METHOD(RequestPlaneTracking);
  static NAN_METHOD(RequestHandTracking);
  static NAN_METHOD(RequestEyeTracking);
  static NAN_METHOD(RequestDepthPopulation);
  static NAN_METHOD(RequestCamera);
  static NAN_METHOD(CancelCamera);
  static NAN_METHOD(RequestCameraMeshing);
  static NAN_METHOD(PrePollEvents);
  static NAN_METHOD(PostPollEvents);

// protected:
  // EGL
  NATIVEwindow *window;
  WebGLRenderingContext *gl;

  // tracking
  MLHandle graphics_client;
  GLuint framebuffer_id;
  MLHandle frame_handle;
  MLHandle head_tracker;
  MLHeadTrackingStaticData head_static_data;
  MLGraphicsVirtualCameraInfoArray virtual_camera_array;

  // position
  std::mutex positionMutex;
  MLVec3f position;
  MLQuaternionf rotation;

  // input
  MLHandle inputTracker;

  // meshing
  GLuint meshVao;
  GLuint meshProgram;
  GLint meshPositionLocation;
  GLint meshModelViewMatrixLocation;
  GLint meshProjectionMatrixLocation;

  // camera content
  GLuint cameraContentFbo;
  GLuint cameraContentVao;
  GLuint cameraContentProgram;
  GLint cameraContentPointLocation;
  GLint cameraContentUvLocation;
  GLint cameraContentCameraInTextureLocation;
  GLint cameraContentContentTextureLocation;
  GLuint cameraContentCameraInTexture;
  GLuint cameraContentContentTexture;
  GLuint cameraContentCameraOutTexture;

  // camera raw
  GLuint cameraRawFbo;
  GLuint cameraRawVao;
  GLuint cameraRawProgram;
  GLint cameraRawPointLocation;
  GLint cameraRawUvLocation;
  GLint cameraRawCameraInTextureLocation;
  GLuint cameraRawCameraInTexture;

  // camera mesh
  GLuint cameraMeshVao2;
  GLuint cameraMeshProgram2;
  GLuint cameraMeshPositionLocation2;
  GLuint cameraMeshUvLocation2;
  GLuint cameraMeshModelViewMatrixLocation2;
  GLuint cameraMeshProjectionMatrixLocation2;
  GLuint cameraMeshCameraSnapshotTextureLocation;

  // texture encode
  GLuint textureEncodeFbo;

  // occlusion
  // MLHandle occlusionTracker;
};

}

Handle<Object> makeMl();

#endif

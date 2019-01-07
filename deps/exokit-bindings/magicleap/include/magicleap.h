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
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
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
#include <ml_image_tracking.h>
#include <ml_eye_tracking.h>
#include <ml_privilege_ids.h>
#include <ml_privilege_functions.h>
#include <ml_input.h>
#include <ml_raycast.h>
#include <ml_gesture.h>
#include <ml_lifecycle.h>
#include <ml_logging.h>

#include "sjpeg.h"

using namespace v8;
using namespace node;

#define MAX_NUM_PLANES (32)
#define PLANE_ENTRY_SIZE (3 + 4 + 2 + 1)
#define CONTROLLER_ENTRY_SIZE (3 + 4 + 6)
#define CAMERA_REQUEST_PLANE_BUFFER_SIZE (5 * 1024 * 1024)

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

class MLPoll {
public:
  Nan::Persistent<Object> windowObj;
  std::function<void()> cb;
  
  MLPoll(Local<Object> windowObj, std::function<void()> cb);
  ~MLPoll();
};

class MLRaycaster : public ObjectWrap {
public:
  MLRaycaster(Local<Object> windowObj, MLHandle requestHandle, Local<Function> cb);
  ~MLRaycaster();

  bool Update();

// protected:
  Nan::Persistent<Object> windowObj;
  MLHandle requestHandle;
  Nan::Persistent<Function> cb;
};

class MeshBuffer {
public:
  MeshBuffer(GLuint positionBuffer, GLuint normalBuffer, GLuint indexBuffer);
  MeshBuffer(const MeshBuffer &meshBuffer);
  MeshBuffer();
  void setBuffers(float *positions, uint32_t numPositions, float *normals, uint16_t *indices, uint16_t numIndices, bool isNew, bool isUnchanged);

  GLuint positionBuffer;
  GLuint normalBuffer;
  GLuint indexBuffer;
  float *positions;
  uint32_t numPositions;
  float *normals;
  uint16_t *indices;
  uint16_t numIndices;
  bool isNew;
  bool isUnchanged;
};

enum class MLUpdateType {
  NEW,
  UNCHANGED,
  UPDATE,
  REMOVE,
};

class MLMesher : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLMesher(Local<Object> windowObj);
  ~MLMesher();

  static NAN_METHOD(New);
  static NAN_GETTER(OnMeshGetter);
  static NAN_SETTER(OnMeshSetter);
  static NAN_METHOD(Destroy);

  void Update();

// protected:
  Nan::Persistent<Object> windowObj;
  Nan::Persistent<Function> cb;
};

class MLPlaneTracker : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLPlaneTracker(Local<Object> windowObj);
  ~MLPlaneTracker();

  static NAN_METHOD(New);
  static NAN_GETTER(OnPlanesGetter);
  static NAN_SETTER(OnPlanesSetter);
  static NAN_METHOD(Destroy);

  void Update();

// protected:
  Nan::Persistent<Object> windowObj;
  Nan::Persistent<Function> cb;
};

class MLHandTracker : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLHandTracker(Local<Object> windowObj);
  ~MLHandTracker();

  static NAN_METHOD(New);
  static NAN_GETTER(OnHandsGetter);
  static NAN_SETTER(OnHandsSetter);
  static NAN_GETTER(OnGestureGetter);
  static NAN_SETTER(OnGestureSetter);
  static NAN_METHOD(Destroy);

  void Update();

// protected:
  Nan::Persistent<Object> windowObj;
  Nan::Persistent<Function> cb;
  Nan::Persistent<Function> ongesture;
};

class MLEyeTracker : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLEyeTracker(Local<Object> windowObj);
  ~MLEyeTracker();

  static NAN_METHOD(New);
  static NAN_GETTER(FixationGetter);
  static NAN_GETTER(EyesGetter);
  static NAN_METHOD(Destroy);

  void Update(MLSnapshot *snapshot);

// protected:
  Nan::Persistent<Object> windowObj;
  MLTransform transform;
  MLTransform leftTransform;
  bool leftBlink;
  MLTransform rightTransform;
  bool rightBlink;
};

class CameraRequest {
public:
  CameraRequest(Local<Function> cbFn);
  void Set(int width, int height, uint8_t *data, size_t size);
  void Update();

// protected:
  Nan::Persistent<Function> cbFn;
  int width;
  int height;
  Nan::Persistent<ArrayBuffer> data;
};

class MLImageTracker : public ObjectWrap {
public:
  static Local<Function> Initialize(Isolate *isolate);

  MLImageTracker(Local<Object> windowObj, MLHandle trackerHandle, float size);
  ~MLImageTracker();

  static NAN_METHOD(New);
  static NAN_GETTER(OnTrackGetter);
  static NAN_SETTER(OnTrackSetter);
  static NAN_METHOD(Destroy);

  void Update(MLSnapshot *snapshot);

// protected:
  Nan::Persistent<Object> windowObj;
  MLHandle trackerHandle;
  float size;
  Nan::Persistent<Function> cb;
  MLTransform transform;
  bool valid;
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
  static NAN_METHOD(RequestHitTest);
  static NAN_METHOD(RequestHandTracking);
  static NAN_METHOD(RequestEyeTracking);
  static NAN_METHOD(RequestImageTracking);
  static NAN_METHOD(RequestDepthPopulation);
  static NAN_METHOD(RequestCamera);
  static NAN_METHOD(CancelCamera);
  static NAN_METHOD(Update);
  static NAN_METHOD(Poll);

  void TickFloor();
  static MLVec3f OffsetFloor(const MLVec3f &position);

// protected:
  // EGL
  NATIVEwindow *window;

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
  GLuint meshVertex;
  GLuint meshFragment;
  GLuint meshProgram;
  GLint positionLocation;
  // GLint normalLocation;
  GLint modelViewMatrixLocation;
  GLint projectionMatrixLocation;

  // camera
  GLuint cameraVao;
  GLuint cameraVertex;
  GLuint cameraFragment;
  GLuint cameraProgram;
  GLint pointLocation;
  GLint uvLocation;
  GLint cameraInTextureLocation;
  GLint contentTextureLocation;
  GLuint pointBuffer;
  GLuint uvBuffer;

  GLuint cameraInTexture;
  GLuint contentTexture;
  GLuint cameraOutTexture;
  GLuint cameraFbo;

  // occlusion
  // MLHandle occlusionTracker;
};

}

Handle<Object> makeMl();

#endif

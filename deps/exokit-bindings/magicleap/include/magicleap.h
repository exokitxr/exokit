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
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <webgl.h>
#include <egl.h>
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

using namespace v8;
using namespace node;

#define MAX_NUM_PLANES (32)
#define PLANE_ENTRY_SIZE (3 + 4 + 2 + 1)
#define CONTROLLER_ENTRY_SIZE (3 + 4 + 3)
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

class HandRequest {
public:
  HandRequest(Local<Function> cbFn);
  void Poll();

// protected:
  Nan::Persistent<Function> cbFn;
};

class MeshBuffer {
public:
  MeshBuffer(GLuint positionBuffer, GLuint normalBuffer, GLuint indexBuffer);
  MeshBuffer(const MeshBuffer &meshBuffer);
  MeshBuffer();
  void setBuffers(float *positions, uint32_t numPositions, float *normals, unsigned short *indices, uint16_t numIndices, bool isNew);

  GLuint positionBuffer;
  GLuint normalBuffer;
  GLuint indexBuffer;
  uint32_t numPositions;
  uint16_t numIndices;
  bool isNew;
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

// protected:
  Nan::Persistent<Function> cb;
};

class PlanesRequest {
public:
  PlanesRequest(Local<Function> cbFn);
  void Poll();

// protected:
  Nan::Persistent<Function> cbFn;
};

class EyeRequest {
public:
  EyeRequest(Local<Function> cbFn);
  void Poll(MLSnapshot *snapshot);

// protected:
  Nan::Persistent<Function> cbFn;
};

class CameraRequestPlane {
public:
  CameraRequestPlane(uint32_t width, uint32_t height, uint8_t *data, uint32_t size, uint32_t bpp, uint32_t stride);
  CameraRequestPlane(MLHandle output);
  void set(uint32_t width, uint32_t height, uint8_t *data, uint32_t size, uint32_t bpp, uint32_t stride);
  void set(MLHandle output);

  uint32_t width;
  uint32_t height;
  uint8_t data[CAMERA_REQUEST_PLANE_BUFFER_SIZE];
  uint32_t size;
  uint32_t bpp;
  uint32_t stride;
};

class CameraRequest {
public:
  CameraRequest(MLHandle request, Local<Function> cbFn);
  void Set(const MLCameraOutput *output);
  void Set(MLHandle output);
  void Poll(WebGLRenderingContext *gl, GLuint fbo, unsigned int width, unsigned int height);

// protected:
  MLHandle request;
  Nan::Persistent<Function> cbFn;
  std::vector<CameraRequestPlane *> planes;
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
  static NAN_METHOD(RequestHand);
  static NAN_METHOD(CancelHand);
  static NAN_METHOD(RequestMeshing);
  static NAN_METHOD(PopulateDepth);
  static NAN_METHOD(RequestPlanes);
  static NAN_METHOD(CancelPlanes);
  static NAN_METHOD(RequestEye);
  static NAN_METHOD(CancelEye);
  static NAN_METHOD(RequestCamera);
  static NAN_METHOD(CancelCamera);
  static NAN_METHOD(PrePollEvents);
  static NAN_METHOD(PostPollEvents);

// protected:
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

  // occlusion
  // MLHandle occlusionTracker;
};

}

Handle<Object> makeMl();

#endif

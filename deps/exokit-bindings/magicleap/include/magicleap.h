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
#include <cmath>
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
#include <ml_planes.h>
#include <ml_privilege_ids.h>
#include <ml_privilege_functions.h>
#include <ml_meshing2.h>
#include <ml_planes.h>
#include <ml_camera.h>
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

struct application_context_t {
  int dummy_value;
};

class MLContext;

class HandRequest {
public:
  HandRequest(Local<Function> cbFn);
  void Poll();

// protected:
  Nan::Persistent<Function> cbFn;
};

class MeshRequest {
public:
  MeshRequest(Local<Function> cbFn);
  void Poll();

// protected:
  Nan::Persistent<Function> cbFn;
};

class PlanesRequest {
public:
  PlanesRequest(Local<Function> cbFn);
  void Poll();

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

class MLStageGeometry : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  MLStageGeometry(MLContext *mlContext);
  ~MLStageGeometry();

  static NAN_METHOD(New);
  // static NAN_METHOD(GetGeometry);

  MLContext *mlContext;
};

class MLContext : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

// protected:
  MLContext();
  ~MLContext();

  static NAN_METHOD(New);
  static NAN_METHOD(InitLifecycle);
  static NAN_METHOD(Present);
  static NAN_METHOD(WaitGetPoses);
  static NAN_METHOD(SubmitFrame);
  static NAN_METHOD(IsPresent);
  static NAN_METHOD(IsSimulated);
  static NAN_METHOD(OnPresentChange);
  static NAN_METHOD(RequestHand);
  static NAN_METHOD(RequestMesh);
  static NAN_METHOD(RequestPlanes);
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
  // MLHandle gestureTracker;

  /* // planes
  MLHandle planesFloorHandle;
  MLHandle planesWallHandle;
  MLHandle planesCeilingHandle;

  MLHandle planesFloorQuery;
  MLHandle planesWallQuery;
  MLHandle planesCeilingQuery;

  MLHandle planesFloorQueryHandle;
  MLHandle planesWallQueryHandle;
  MLHandle planesCeilingQueryHandle;

  MLPlane floorPlanes[MAX_NUM_PLANES];
  MLPlane wallPlanes[MAX_NUM_PLANES];
  MLPlane ceilingPlanes[MAX_NUM_PLANES];
  uint32_t numFloorPlanes;
  uint32_t numWallPlanes;
  uint32_t numCeilingPlanes;

  std::condition_variable mesherCv;
  std::mutex mesherMutex;

  bool haveMeshStaticData;
  MLMeshingStaticData meshStaticData;
  MLDataArray meshData;
  MLDataArrayDiff meshesDataDiff;
  MLDataArray meshData2;
  MLDataArrayDiff meshesDataDiff2;
  std::vector<uint8_t> positions;
  std::vector<uint8_t> normals;
  std::vector<uint8_t> triangles; */

  // occlusion
  // MLHandle occlusionTracker;
};

}

Handle<Object> makeMl();

#endif

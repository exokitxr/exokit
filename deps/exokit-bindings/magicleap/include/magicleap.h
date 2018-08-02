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
#include <nan/nan.h>
#include <defines.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <glfw.h>
#include <ml_graphics.h>
#include <ml_head_tracking.h>
#include <ml_perception.h>
#include <ml_planes.h>
#include <ml_privilege_ids.h>
#include <ml_privilege_functions.h>
#include <ml_meshing.h>
#include <ml_input.h>
#include <ml_gesture.h>
#include <ml_lifecycle.h>
#include <ml_logging.h>

using namespace v8;
using namespace node;

#define MAX_NUM_PLANES (32)
#define PLANE_ENTRY_SIZE (3 + 4 + 2 + 1)
#define CONTROLLER_ENTRY_SIZE (3 + 4 + 1)

namespace ml {

struct application_context_t {
  int dummy_value;
};

class MLContext;

class MLStageGeometry : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  MLStageGeometry(MLContext *mlContext);
  ~MLStageGeometry();
  
  static NAN_METHOD(New);
  static NAN_METHOD(GetGeometry);
  
  MLContext *mlContext;
};

class MLContext : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  MLContext();
  ~MLContext();

  static NAN_METHOD(New);
  static NAN_METHOD(InitLifecycle);
  static NAN_METHOD(Present);
  static NAN_METHOD(WaitGetPoses);
  static NAN_METHOD(SubmitFrame);
  static NAN_METHOD(IsPresent);
  static NAN_METHOD(OnPresentChange);

protected:
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
  MLHandle gestureTracker;

  // planes
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

  // meshing
  MLHandle meshTracker;
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
  std::vector<uint8_t> triangles;

  // occlusion
  // MLHandle occlusionTracker;

  friend class MLStageGeometry;
};

}

Handle<Object> makeMl();

#endif

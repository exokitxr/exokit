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
#include <glfw.h>
#include <ml_graphics.h>
#include <ml_head_tracking.h>
#include <ml_perception.h>
#include <ml_planes.h>
#include <ml_input.h>
#include <ml_gesture.h>
#include <ml_lifecycle.h>
#include <ml_logging.h>

using namespace v8;
using namespace node;

#define MAX_NUM_PLANES (32)
#define PLANE_ENTRY_SIZE (3 + 4 + 2 + 1)
#define CONTROLLER_ENTRY_SIZE (3 + 4)

namespace ml {

struct application_context_t {
  int dummy_value;
};

class MLContext : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);

protected:
  MLContext();
  ~MLContext();

  static NAN_METHOD(New);
  static NAN_METHOD(Init);
  static NAN_METHOD(WaitGetPoses);
  static NAN_METHOD(SubmitFrame);
  static NAN_METHOD(IsPresent);
  static NAN_METHOD(OnPresentChange);
  static void LifecycleInit();

protected:
  MLHandle graphics_client;
  GLuint framebuffer_id;
  MLHandle frame_handle;
  MLHandle head_tracker;
  MLHeadTrackingStaticData head_static_data;
  MLGraphicsVirtualCameraInfoArray virtual_camera_array;

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

  MLHandle inputTracker;
  MLHandle gestureTracker;
};

}

Handle<Object> makeMl();

#endif

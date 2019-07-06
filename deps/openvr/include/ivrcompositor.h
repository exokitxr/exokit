#ifndef _OPENVR_IVRCOMPOSITOR_H_
#define _OPENVR_IVRCOMPOSITOR_H_

#include <nan.h>
#include <v8.h>

#include <webgl.h>
#include <windowsystem.h>

// Forward declaration of OpenVR class.
// We only need the pointer here, so this is cleaner than importing the header.
namespace vr
{
class IVRCompositor;
}

namespace vr {
  class VRPoseRes;

  void RunResInMainThread(uv_async_t *handle);
}

class VRPoseRes {
public:
  VRPoseRes(Local<Function> cb);
  ~VRPoseRes();

  Nan::Persistent<Function> cb;
};

class IVRCompositor : public Nan::ObjectWrap
{
public:
  static NAN_MODULE_INIT(Init);

  // Static factory construction method for other node addons to use.
  static v8::Local<v8::Object> NewInstance(vr::IVRCompositor *compositor);

private:
  explicit IVRCompositor(vr::IVRCompositor *self);
  ~IVRCompositor() = default;

  // Node construction method for new instances.
  static NAN_METHOD(New);

  static NAN_METHOD(RequestGetPoses);
  // static NAN_METHOD(WaitGetPoses);
  static NAN_METHOD(Submit);

  /// Create a singleton reference to a constructor function.
  static inline Nan::Persistent<v8::Function>& constructor()
  {
    static Nan::Persistent<v8::Function> the_constructor;
    return the_constructor;
  }

  /// Reference to wrapped OpenVR instance.
  vr::IVRCompositor * const self_;
  vr::VRActionHandle_t leftHandActionHandle;
  vr::VRActionHandle_t leftHandAnimActionHandle;
  vr::VRActionHandle_t rightHandActionHandle;
  vr::VRActionHandle_t rightHandAnimActionHandle;
  vr::VRActionSetHandle_t actionSetHandle;
};

NAN_METHOD(NewCompositor);

#endif

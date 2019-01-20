#ifndef _OPENVR_IVRCOMPOSITOR_H_
#define _OPENVR_IVRCOMPOSITOR_H_

#include <nan.h>
#include <v8.h>

#include <webgl.h>

// Forward declaration of OpenVR class.
// We only need the pointer here, so this is cleaner than importing the header.
namespace vr
{
class IVRCompositor;
}

namespace vr {
  class VRPoseRes;

  void RunResInMainThread(uv_async_t *handle);

  extern uv_sem_t reqSem;
  extern uv_sem_t resSem;
  extern uv_async_t resAsync;
  extern std::mutex reqMutex;
  extern std::mutex resMutex;
  extern std::deque<std::function<void()>> reqCbs;
  extern std::deque<VRPoseRes *> resCbs;
  extern std::thread reqThead;
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

  static NAN_METHOD(WaitGetPoses);
  static NAN_METHOD(Submit);

  /// Create a singleton reference to a constructor function.
  static inline Nan::Persistent<v8::Function>& constructor()
  {
    static Nan::Persistent<v8::Function> the_constructor;
    return the_constructor;
  }

  /// Reference to wrapped OpenVR instance.
  vr::IVRCompositor * const self_;
};

NAN_METHOD(NewCompositor);

#endif

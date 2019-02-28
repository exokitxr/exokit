#ifndef _OVR_SESSION_H_
#define _OVR_SESSION_H_

#include <nan.h>
#include <v8.h>

#include "OVR_CAPI_GL.h"

class OVRSession : public Nan::ObjectWrap
{
public:
  static NAN_MODULE_INIT(Init);

  // Static factory construction method for other node addons to use.
  static v8::Local<v8::Object> NewInstance(ovrSession *session);

private:
  explicit OVRSession(ovrSession *session);
  ~OVRSession() = default;

  // Node construction method for new instances.
  static NAN_METHOD(New);
  static NAN_METHOD(SetupSwapChain);

  /// Create a singleton reference to a constructor function.
  static inline Nan::Persistent<v8::Function>& constructor()
  {
    static Nan::Persistent<v8::Function> the_constructor;
    return the_constructor;
  }

  /// Reference to wrapped ovrSession instance.
  ovrSession * self_;
};

#endif

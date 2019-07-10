#include <ivrsystem.h>
#include <openvr-util.h>

#include <array>
#include <node.h>
#include <openvr.h>

#include <defines.h>

using namespace v8;

using TrackedDevicePoseArray = std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount>;
using TrackedDeviceIndexArray = std::array<vr::TrackedDeviceIndex_t, vr::k_unMaxTrackedDeviceCount>;

//=============================================================================
NAN_MODULE_INIT(IVRSystem::Init)
{
  // Create a function template that is called in JS to create this wrapper.
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);

  // Declare human-readable name for this wrapper.
  tpl->SetClassName(Nan::New("IVRSystem").ToLocalChecked());

  // Declare the stored number of fields (just the wrapped C++ object).
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Assign all the wrapped methods of this object.
  Nan::SetPrototypeMethod(tpl, "GetRecommendedRenderTargetSize", GetRecommendedRenderTargetSize);
  Nan::SetPrototypeMethod(tpl, "GetProjectionMatrix", GetProjectionMatrix);
  Nan::SetPrototypeMethod(tpl, "GetProjectionRaw", GetProjectionRaw);
  Nan::SetPrototypeMethod(tpl, "ComputeDistortion", ComputeDistortion);
  Nan::SetPrototypeMethod(tpl, "GetEyeToHeadTransform", GetEyeToHeadTransform);
  Nan::SetPrototypeMethod(tpl, "GetTimeSinceLastVsync", GetTimeSinceLastVsync);
  Nan::SetPrototypeMethod(tpl, "GetD3D9AdapterIndex", GetD3D9AdapterIndex);
  Nan::SetPrototypeMethod(tpl, "GetDXGIOutputInfo", GetDXGIOutputInfo);
  Nan::SetPrototypeMethod(tpl, "IsDisplayOnDesktop", IsDisplayOnDesktop);
  Nan::SetPrototypeMethod(tpl, "SetDisplayVisibility", SetDisplayVisibility);
  Nan::SetPrototypeMethod(tpl, "GetDeviceToAbsoluteTrackingPose", GetDeviceToAbsoluteTrackingPose);
  Nan::SetPrototypeMethod(tpl, "ResetSeatedZeroPose", ResetSeatedZeroPose);
  Nan::SetPrototypeMethod(tpl, "GetSeatedZeroPoseToStandingAbsoluteTrackingPose", GetSeatedZeroPoseToStandingAbsoluteTrackingPose);
  Nan::SetPrototypeMethod(tpl, "GetRawZeroPoseToStandingAbsoluteTrackingPose", GetRawZeroPoseToStandingAbsoluteTrackingPose);
  Nan::SetPrototypeMethod(tpl, "GetSortedTrackedDeviceIndicesOfClass", GetSortedTrackedDeviceIndicesOfClass);
  Nan::SetPrototypeMethod(tpl, "GetTrackedDeviceActivityLevel", GetTrackedDeviceActivityLevel);
  Nan::SetPrototypeMethod(tpl, "ApplyTransform", ApplyTransform);

  /// virtual vr::TrackedDeviceIndex_t GetTrackedDeviceIndexForControllerRole( vr::ETrackedControllerRole unDeviceType ) = 0;
  Nan::SetPrototypeMethod(tpl, "GetTrackedDeviceIndexForControllerRole", GetTrackedDeviceIndexForControllerRole);
  
  /// virtual vr::ETrackedControllerRole GetControllerRoleForTrackedDeviceIndex( vr::TrackedDeviceIndex_t unDeviceIndex ) = 0;

  Nan::SetPrototypeMethod(tpl, "GetTrackedDeviceClass", GetTrackedDeviceClass);
  Nan::SetPrototypeMethod(tpl, "GetControllerState", GetControllerState);

  /// virtual bool IsTrackedDeviceConnected( vr::TrackedDeviceIndex_t unDeviceIndex ) = 0;
  /// virtual bool GetBoolTrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L ) = 0;
  /// virtual float GetFloatTrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L ) = 0;
  /// virtual int32_t GetInt32TrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L ) = 0;
  /// virtual uint64_t GetUint64TrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L ) = 0;
  /// virtual HmdMatrix34_t GetMatrix34TrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L ) = 0;
  /// virtual uint32_t GetStringTrackedDeviceProperty( vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize, ETrackedPropertyError *pError = 0L ) = 0;
  /// virtual const char *GetPropErrorNameFromEnum( ETrackedPropertyError error ) = 0;
  /// virtual bool PollNextEvent( VREvent_t *pEvent, uint32_t uncbVREvent ) = 0;
  /// virtual bool PollNextEventWithPose( ETrackingUniverseOrigin eOrigin, VREvent_t *pEvent, uint32_t uncbVREvent, vr::TrackedDevicePose_t *pTrackedDevicePose ) = 0;
  /// virtual const char *GetEventTypeNameFromEnum( EVREventType eType ) = 0;
  /// virtual HiddenAreaMesh_t GetHiddenAreaMesh( EVREye eEye ) = 0;
  /// virtual bool GetControllerState( vr::TrackedDeviceIndex_t unControllerDeviceIndex, vr::VRControllerState_t *pControllerState ) = 0;
  /// virtual bool GetControllerStateWithPose( ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex, vr::VRControllerState_t *pControllerState, TrackedDevicePose_t *pTrackedDevicePose ) = 0;
  // virtual void TriggerHapticPulse( vr::TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec ) = 0;
  Nan::SetPrototypeMethod(tpl, "TriggerHapticPulse", TriggerHapticPulse);
  /// virtual const char *GetButtonIdNameFromEnum( EVRButtonId eButtonId ) = 0;
  /// virtual const char *GetControllerAxisTypeNameFromEnum( EVRControllerAxisType eAxisType ) = 0;

  /* Nan::SetPrototypeMethod(tpl, "CaptureInputFocus", CaptureInputFocus);
  Nan::SetPrototypeMethod(tpl, "ReleaseInputFocus", ReleaseInputFocus);
  Nan::SetPrototypeMethod(tpl, "IsInputFocusCapturedByAnotherProcess", IsInputFocusCapturedByAnotherProcess); */

  /// virtual uint32_t DriverDebugRequest( vr::TrackedDeviceIndex_t unDeviceIndex, const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) = 0;
  /// virtual vr::EVRFirmwareError PerformFirmwareUpdate( vr::TrackedDeviceIndex_t unDeviceIndex ) = 0;

  Nan::SetPrototypeMethod(tpl, "AcknowledgeQuit_Exiting", AcknowledgeQuit_Exiting);
  Nan::SetPrototypeMethod(tpl, "AcknowledgeQuit_UserPrompt", AcknowledgeQuit_UserPrompt);

  // Set a static constructor function to reference the `New` function template.
  constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
}

//=============================================================================
Local<Object> IVRSystem::NewInstance(vr::IVRSystem *system)
{
  Nan::EscapableHandleScope scope;
  Local<Function> cons = Nan::New(constructor());
  Local<Value> argv[1] = { Nan::New<External>(system) };
  return scope.Escape(Nan::NewInstance(cons, 1, argv).ToLocalChecked());
}

//=============================================================================
IVRSystem::IVRSystem(vr::IVRSystem *self)
: self_(self)
{
  // Do nothing.
}

//=============================================================================
NAN_METHOD(IVRSystem::New)
{
  if (!info.IsConstructCall())
  {
    Nan::ThrowError("Use the `new` keyword when creating a new instance.");
    return;
  }

  if (info.Length() != 1 || !info[0]->IsExternal())
  {
    Nan::ThrowTypeError("Argument[0] must be an `IVRSystem*`.");
    return;
  }

  auto wrapped_instance = static_cast<vr::IVRSystem*>(
    Local<External>::Cast(info[0])->Value());
  IVRSystem *obj = new IVRSystem(wrapped_instance);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

//=============================================================================
// virtual void GetRecommendedRenderTargetSize( uint32_t *pnWidth, uint32_t *pnHeight ) = 0;
NAN_METHOD(IVRSystem::GetRecommendedRenderTargetSize)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  uint32_t nWidth, nHeight;
  obj->self_->GetRecommendedRenderTargetSize(&nWidth, &nHeight);

  Local<Object> result = Nan::New<Object>();
  {
    Local<String> width_prop = Nan::New<String>("width").ToLocalChecked();
    Nan::Set(result, width_prop, Nan::New<Number>(nWidth));

    Local<String> height_prop = Nan::New<String>("height").ToLocalChecked();
    Nan::Set(result, height_prop, Nan::New<Number>(nHeight));
  }
  info.GetReturnValue().Set(result);
}

//=============================================================================
/// virtual HmdMatrix44_t GetProjectionMatrix( EVREye eEye, float fNearZ, float fFarZ ) = 0;
NAN_METHOD(IVRSystem::GetProjectionMatrix)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 4)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number (EVREye).");
    return;
  }

  uint32_t nEye = TO_UINT32(info[0]);
  if (nEye >= 2)
  {
    Nan::ThrowTypeError("Argument[0] was out of enum range (EVREye).");
    return;
  }

  if (!info[1]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[1] must be a number.");
    return;
  }

  if (!info[2]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[2] must be a number.");
    return;
  }

  if (!info[3]->IsFloat32Array())
  {
    Nan::ThrowTypeError("Argument[3] must be a Float32Array.");
    return;
  }

  vr::EVREye eEye = static_cast<vr::EVREye>(nEye);
  float fNearZ = TO_FLOAT(info[1]);
  float fFarZ = TO_FLOAT(info[2]);
  vr::HmdMatrix44_t matrix = obj->self_->GetProjectionMatrix(eEye, fNearZ, fFarZ);

  Local<Float32Array> float32Array = Local<Float32Array>::Cast(info[3]);
  for (unsigned int v = 0; v < 4; v++) {
    for (unsigned int u = 0; u < 4; u++) {
      float32Array->Set(v * 4 + u, Number::New(Isolate::GetCurrent(), matrix.m[u][v]));
    }
  }
}

//=============================================================================
/// virtual void GetProjectionRaw( EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom ) = 0;
NAN_METHOD(IVRSystem::GetProjectionRaw)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 2)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number (EVREye).");
    return;
  }

  uint32_t nEye = TO_UINT32(info[0]);
  if (nEye >= 2)
  {
    Nan::ThrowTypeError("Argument[0] was out of enum range (EVREye).");
    return;
  }

  vr::EVREye eEye = static_cast<vr::EVREye>(nEye);
  float fLeft, fRight, fTop, fBottom;
  obj->self_->GetProjectionRaw(eEye, &fLeft, &fRight, &fTop, &fBottom);

  Local<Float32Array> float32Array = Local<Float32Array>::Cast(info[1]);
  float32Array->Set(0, Nan::New<Number>(fLeft));
  float32Array->Set(1, Nan::New<Number>(fRight));
  float32Array->Set(2, Nan::New<Number>(fTop));
  float32Array->Set(3, Nan::New<Number>(fBottom));
}

//=============================================================================
/// virtual bool ComputeDistortion( EVREye eEye, float fU, float fV, DistortionCoordinates_t *pDistortionCoordinates ) = 0;
NAN_METHOD(IVRSystem::ComputeDistortion)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 3)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number (EVREye).");
    return;
  }

  uint32_t nEye = TO_UINT32(info[0]);
  if (nEye >= 2)
  {
    Nan::ThrowTypeError("Argument[0] was out of enum range (EVREye).");
    return;
  }

  if (!info[1]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[1] must be a number.");
    return;
  }

  if (!info[2]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[2] must be a number.");
    return;
  }

  vr::EVREye eEye = static_cast<vr::EVREye>(nEye);
  float fU = TO_FLOAT(info[1]);
  float fV = TO_FLOAT(info[2]);
  vr::DistortionCoordinates_t distortionCoordinates;
  bool success = obj->self_->ComputeDistortion(eEye, fU, fV, &distortionCoordinates);

  if (!success)
  {
    Nan::ThrowError("Distortion coordinates not suitable.");
    return;
  }
  info.GetReturnValue().Set(encode(distortionCoordinates));
}

//=============================================================================
/// virtual HmdMatrix34_t GetEyeToHeadTransform( EVREye eEye ) = 0;
NAN_METHOD(IVRSystem::GetEyeToHeadTransform)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 2)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number (EVREye).");
    return;
  }

  if (!info[1]->IsFloat32Array())
  {
    Nan::ThrowTypeError("Argument[1] must be a Float32Array.");
    return;
  }

  uint32_t nEye = TO_UINT32(info[0]);
  if (nEye >= 2)
  {
    Nan::ThrowTypeError("Argument[0] was out of enum range (EVREye).");
    return;
  }

  vr::EVREye eEye = static_cast<vr::EVREye>(nEye);
  vr::HmdMatrix34_t matrix = obj->self_->GetEyeToHeadTransform(eEye);

  Local<Float32Array> float32Array = Local<Float32Array>::Cast(info[1]);
  for (unsigned int v = 0; v < 4; v++) {
    for (unsigned int u = 0; u < 3; u++) {
      float32Array->Set(v * 4 + u, Number::New(Isolate::GetCurrent(), matrix.m[u][v]));
    }
  }
  float32Array->Set(0 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
  float32Array->Set(1 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
  float32Array->Set(2 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
  float32Array->Set(3 * 4 + 3, Number::New(Isolate::GetCurrent(), 1));
}

//=============================================================================
/// virtual bool GetTimeSinceLastVsync( float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter ) = 0;
NAN_METHOD(IVRSystem::GetTimeSinceLastVsync)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  float fSecondsSinceLastVsync;
  uint64_t ulFrameCounter;
  obj->self_->GetTimeSinceLastVsync(&fSecondsSinceLastVsync, &ulFrameCounter);

  Local<Object> result = Nan::New<Object>();
  {
    Local<String> seconds_prop = Nan::New<String>("seconds").ToLocalChecked();
    Nan::Set(result, seconds_prop, Nan::New<Number>(fSecondsSinceLastVsync));

    // We can't return a 64-bit int, so we just have to return a 32-bit
    // truncation and hope that clients can deal with eventual overflow.
    Local<String> frame_prop = Nan::New<String>("frame").ToLocalChecked();
    Nan::Set(result, frame_prop, Nan::New<Number>(static_cast<uint32_t>(ulFrameCounter)));
  }
  info.GetReturnValue().Set(result);
}

//=============================================================================
/// virtual int32_t GetD3D9AdapterIndex() = 0;
NAN_METHOD(IVRSystem::GetD3D9AdapterIndex)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  uint32_t uIndex = obj->self_->GetD3D9AdapterIndex();
  info.GetReturnValue().Set(Nan::New<Number>(uIndex));
}

//=============================================================================
/// virtual void GetDXGIOutputInfo( int32_t *pnAdapterIndex ) = 0;
NAN_METHOD(IVRSystem::GetDXGIOutputInfo)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  int32_t nAdapterIndex;
  obj->self_->GetDXGIOutputInfo(&nAdapterIndex);
  info.GetReturnValue().Set(Nan::New<Number>(nAdapterIndex));
}

//=============================================================================
/// virtual bool IsDisplayOnDesktop() = 0;
NAN_METHOD(IVRSystem::IsDisplayOnDesktop)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  bool bIsVisibleOnDesktop = obj->self_->IsDisplayOnDesktop();
  info.GetReturnValue().Set(Nan::New<Boolean>(bIsVisibleOnDesktop));
}

//=============================================================================
/// virtual bool SetDisplayVisibility( bool bIsVisibleOnDesktop ) = 0;
NAN_METHOD(IVRSystem::SetDisplayVisibility)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 1)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsBoolean())
  {
    Nan::ThrowTypeError("Argument[0] must be a boolean.");
    return;
  }

  bool bIsVisibleOnDesktop = TO_BOOL(info[0]);
  bool bSuccess = obj->self_->SetDisplayVisibility(bIsVisibleOnDesktop);
  info.GetReturnValue().Set(Nan::New<Boolean>(bSuccess));
}

//=============================================================================
/// virtual void GetDeviceToAbsoluteTrackingPose( ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow, VR_ARRAY_COUNT(unTrackedDevicePoseArrayCount) TrackedDevicePose_t *pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount ) = 0;
NAN_METHOD(IVRSystem::GetDeviceToAbsoluteTrackingPose)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 4)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number (ETrackingUniverseOrigin).");
    return;
  }

  uint32_t nOrigin = TO_UINT32(info[0]);
  if (nOrigin >= 3)
  {
    Nan::ThrowTypeError("Argument[0] was out of enum range (ETrackingUniverseOrigin).");
    return;
  }

  if (!info[1]->IsFloat32Array() || !info[2]->IsFloat32Array() || !info[3]->IsFloat32Array())
  {
    Nan::ThrowTypeError("Arguments[1-3] must be Float32Array.");
    return;
  }

  vr::ETrackingUniverseOrigin eOrigin = static_cast<vr::ETrackingUniverseOrigin>(nOrigin);

  float fSecondsSinceLastVsync;
  obj->self_->GetTimeSinceLastVsync( &fSecondsSinceLastVsync, NULL );
  const float fDisplayFrequency = obj->self_->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float );
  const float fFrameDuration = 1.f / fDisplayFrequency;
  const float fVsyncToPhotons = obj->self_->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SecondsFromVsyncToPhotons_Float );
  const float fPredictedSecondsToPhotonsFromNow = fFrameDuration - fSecondsSinceLastVsync + fVsyncToPhotons;

  TrackedDevicePoseArray trackedDevicePoseArray;
  obj->self_->GetDeviceToAbsoluteTrackingPose(
    eOrigin, fPredictedSecondsToPhotonsFromNow, trackedDevicePoseArray.data(),
    static_cast<uint32_t>(trackedDevicePoseArray.size())
  );

  Local<Float32Array> hmdFloat32Array = Local<Float32Array>::Cast(info[1]);
  Local<Float32Array> leftControllerFloat32Array = Local<Float32Array>::Cast(info[2]);
  Local<Float32Array> rightControllerFloat32Array = Local<Float32Array>::Cast(info[3]);
  hmdFloat32Array->Set(0, Number::New(Isolate::GetCurrent(), std::numeric_limits<float>::quiet_NaN()));
  leftControllerFloat32Array->Set(0, Number::New(Isolate::GetCurrent(), std::numeric_limits<float>::quiet_NaN()));
  rightControllerFloat32Array->Set(0, Number::New(Isolate::GetCurrent(), std::numeric_limits<float>::quiet_NaN()));

  for (unsigned int i = 0; i < trackedDevicePoseArray.size(); i++) {
    const vr::TrackedDevicePose_t &trackedDevicePose = trackedDevicePoseArray[i];
    if (trackedDevicePose.bPoseIsValid) {
      const vr::ETrackedDeviceClass deviceClass = obj->self_->GetTrackedDeviceClass(i);
      if (deviceClass == vr::TrackedDeviceClass_HMD) {
        const vr::HmdMatrix34_t &matrix = trackedDevicePose.mDeviceToAbsoluteTracking;

        for (unsigned int v = 0; v < 4; v++) {
          for (unsigned int u = 0; u < 3; u++) {
            hmdFloat32Array->Set(v * 4 + u, Number::New(Isolate::GetCurrent(), matrix.m[u][v]));
          }
        }
        hmdFloat32Array->Set(0 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
        hmdFloat32Array->Set(1 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
        hmdFloat32Array->Set(2 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
        hmdFloat32Array->Set(3 * 4 + 3, Number::New(Isolate::GetCurrent(), 1));
      } else if (deviceClass == vr::TrackedDeviceClass_Controller) {
        const vr::ETrackedControllerRole controllerRole = obj->self_->GetControllerRoleForTrackedDeviceIndex(i);
        if (controllerRole == vr::TrackedControllerRole_LeftHand) {
          const vr::HmdMatrix34_t &matrix = trackedDevicePose.mDeviceToAbsoluteTracking;

          for (unsigned int v = 0; v < 4; v++) {
            for (unsigned int u = 0; u < 3; u++) {
              leftControllerFloat32Array->Set(v * 4 + u, Number::New(Isolate::GetCurrent(), matrix.m[u][v]));
            }
          }
          leftControllerFloat32Array->Set(0 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
          leftControllerFloat32Array->Set(1 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
          leftControllerFloat32Array->Set(2 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
          leftControllerFloat32Array->Set(3 * 4 + 3, Number::New(Isolate::GetCurrent(), 1));
        } else if (controllerRole == vr::TrackedControllerRole_LeftHand) {
          const vr::HmdMatrix34_t &matrix = trackedDevicePose.mDeviceToAbsoluteTracking;

          for (unsigned int v = 0; v < 4; v++) {
            for (unsigned int u = 0; u < 3; u++) {
              rightControllerFloat32Array->Set(v * 4 + u, Number::New(Isolate::GetCurrent(), matrix.m[u][v]));
            }
          }
          rightControllerFloat32Array->Set(0 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
          rightControllerFloat32Array->Set(1 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
          rightControllerFloat32Array->Set(2 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
          rightControllerFloat32Array->Set(3 * 4 + 3, Number::New(Isolate::GetCurrent(), 1));
        }
      }
    }
  }
}

//=============================================================================
/// virtual void ResetSeatedZeroPose() = 0;
NAN_METHOD(IVRSystem::ResetSeatedZeroPose)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  obj->self_->ResetSeatedZeroPose();
}

//=============================================================================
/// virtual HmdMatrix34_t GetSeatedZeroPoseToStandingAbsoluteTrackingPose() = 0;
NAN_METHOD(IVRSystem::GetSeatedZeroPoseToStandingAbsoluteTrackingPose)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 1)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsFloat32Array())
  {
    Nan::ThrowTypeError("Argument[0] must be a Float32Array.");
    return;
  }

  const vr::HmdMatrix34_t &matrix = obj->self_->GetSeatedZeroPoseToStandingAbsoluteTrackingPose();
  Local<Float32Array> float32Array = Local<Float32Array>::Cast(info[0]);
  for (unsigned int v = 0; v < 4; v++) {
    for (unsigned int u = 0; u < 3; u++) {
      float32Array->Set(v * 4 + u, Number::New(Isolate::GetCurrent(), matrix.m[u][v]));
    }
  }
  float32Array->Set(0 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
  float32Array->Set(1 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
  float32Array->Set(2 * 4 + 3, Number::New(Isolate::GetCurrent(), 0));
  float32Array->Set(3 * 4 + 3, Number::New(Isolate::GetCurrent(), 1));
}

//=============================================================================
/// virtual HmdMatrix34_t GetRawZeroPoseToStandingAbsoluteTrackingPose() = 0;
NAN_METHOD(IVRSystem::GetRawZeroPoseToStandingAbsoluteTrackingPose)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  vr::HmdMatrix34_t matrix = obj->self_->GetRawZeroPoseToStandingAbsoluteTrackingPose();
  info.GetReturnValue().Set(encode(matrix));
}

//=============================================================================
/// virtual uint32_t GetSortedTrackedDeviceIndicesOfClass( ETrackedDeviceClass eTrackedDeviceClass, VR_ARRAY_COUNT(unTrackedDeviceIndexArrayCount) vr::TrackedDeviceIndex_t *punTrackedDeviceIndexArray, uint32_t unTrackedDeviceIndexArrayCount, vr::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex = k_unTrackedDeviceIndex_Hmd ) = 0;
NAN_METHOD(IVRSystem::GetSortedTrackedDeviceIndicesOfClass)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() < 1 || info.Length() > 2)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number (ETrackedDeviceClass).");
    return;
  }

  uint32_t nTrackedDeviceClass = TO_UINT32(info[0]);
  if (nTrackedDeviceClass >= 6)
  {
    Nan::ThrowTypeError("Argument[0] was out of enum range (ETrackedDeviceClass).");
    return;
  }

  vr::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex = 0;
  if (!info[1]->IsUndefined())
  {
    if (!info[1]->IsNumber())
    {
      Nan::ThrowTypeError("Argument[1] must be a number.");
      return;
    }
    else
    {
      unRelativeToTrackedDeviceIndex = TO_UINT32(info[1]);
    }
  }

  vr::ETrackedDeviceClass eTrackedDeviceClass =
    static_cast<vr::ETrackedDeviceClass>(nTrackedDeviceClass);
  TrackedDeviceIndexArray trackedDeviceIndexArray;
  uint32_t nDeviceIndices = obj->self_->GetSortedTrackedDeviceIndicesOfClass(
    eTrackedDeviceClass, trackedDeviceIndexArray.data(),
    static_cast<uint32_t>(trackedDeviceIndexArray.size()),
    unRelativeToTrackedDeviceIndex
  );

  info.GetReturnValue().Set(encode<(uint32_t)vr::k_unMaxTrackedDeviceCount>(trackedDeviceIndexArray, nDeviceIndices));
}

//=============================================================================
/// virtual EDeviceActivityLevel GetTrackedDeviceActivityLevel( vr::TrackedDeviceIndex_t unDeviceId ) = 0;
NAN_METHOD(IVRSystem::GetTrackedDeviceActivityLevel)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 1)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number.");
    return;
  }

  uint32_t unDeviceId = TO_UINT32(info[0]);
  vr::EDeviceActivityLevel deviceActivityLevel =
    obj->self_->GetTrackedDeviceActivityLevel(unDeviceId);
  info.GetReturnValue().Set(Nan::New<Number>(
    static_cast<uint32_t>(deviceActivityLevel)));
}

//=============================================================================
/// virtual void ApplyTransform( TrackedDevicePose_t *pOutputPose, const TrackedDevicePose_t *pTrackedDevicePose, const HmdMatrix34_t *pTransform ) = 0;
NAN_METHOD(IVRSystem::ApplyTransform)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 2)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsObject())
  {
    Nan::ThrowTypeError("Argument[0] must be a tracked device pose.");
    return;
  }

  if (!info[1]->IsArray())
  {
    Nan::ThrowTypeError("Argument[1] must be a 3x4 matrix.");
    return;
  }

  const auto trackedDevicePose = decode<vr::TrackedDevicePose_t>(info[0]);
  const auto transform = decode<vr::HmdMatrix34_t>(info[1]);
  vr::TrackedDevicePose_t outputPose;
  obj->self_->ApplyTransform(&outputPose, &trackedDevicePose, &transform);
  info.GetReturnValue().Set(encode(outputPose));
}

NAN_METHOD(IVRSystem::GetTrackedDeviceIndexForControllerRole)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 1)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }
  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number.");
    return;
  }

  vr::ETrackedControllerRole role = static_cast<vr::ETrackedControllerRole>(TO_UINT32(info[0]));
  vr::TrackedDeviceIndex_t deviceClass = obj->self_->GetTrackedDeviceIndexForControllerRole(role);
  info.GetReturnValue().Set(Nan::New<Number>(
    static_cast<uint32_t>(deviceClass)));
}

//=============================================================================
/// virtual ETrackedDeviceClass GetTrackedDeviceClass( vr::TrackedDeviceIndex_t unDeviceIndex ) = 0;
NAN_METHOD(IVRSystem::GetTrackedDeviceClass)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 1)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number.");
    return;
  }

  uint32_t unDeviceIndex = TO_UINT32(info[0]);
  vr::ETrackedDeviceClass trackedDeviceClass =
    obj->self_->GetTrackedDeviceClass(unDeviceIndex);
  info.GetReturnValue().Set(Nan::New<Number>(
    static_cast<uint32_t>(trackedDeviceClass)));
}

NAN_METHOD(IVRSystem::GetControllerState)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 2)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number.");
    return;
  }

  if (!info[1]->IsFloat32Array())
  {
    Nan::ThrowTypeError("Argument[1] must be a Float32Array.");
    return;
  }

  uint32_t side = TO_UINT32(info[0]);
  
  for (unsigned int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
    vr::ETrackedDeviceClass deviceClass = obj->self_->GetTrackedDeviceClass(i);
    if (deviceClass == vr::TrackedDeviceClass_Controller) {
      const vr::ETrackedControllerRole controllerRole = obj->self_->GetControllerRoleForTrackedDeviceIndex(i);
      if ((side == 0 && controllerRole == vr::TrackedControllerRole_LeftHand) || (side == 1 && controllerRole == vr::TrackedControllerRole_RightHand)) {
        vr::VRControllerState_t controllerState;
        if (obj->self_->GetControllerState(i, &controllerState, sizeof(controllerState))) {
          Local<Float32Array> buttons = Local<Float32Array>::Cast(info[1]);
          
          buttons->Set(0, Number::New(Isolate::GetCurrent(), 1));

          buttons->Set(1, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_System)) ? 1 : 0));
          buttons->Set(2, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu)) ? 1 : 0));
          buttons->Set(3, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_Grip)) ? 1 : 0));
          buttons->Set(4, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)) ? 1 : 0));
          buttons->Set(5, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)) ? 1 : 0));

          buttons->Set(6, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_System)) ? 1 : 0));
          buttons->Set(7, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu)) ? 1 : 0));
          buttons->Set(8, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_Grip)) ? 1 : 0));
          buttons->Set(9, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)) ? 1 : 0));
          buttons->Set(10, Number::New(Isolate::GetCurrent(), (controllerState.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)) ? 1 : 0));

          buttons->Set(11, Number::New(Isolate::GetCurrent(), controllerState.rAxis[0].x));
          buttons->Set(12, Number::New(Isolate::GetCurrent(), controllerState.rAxis[0].y));
          buttons->Set(13, Number::New(Isolate::GetCurrent(), controllerState.rAxis[1].x));
          buttons->Set(14, Number::New(Isolate::GetCurrent(), controllerState.rAxis[1].y));
          buttons->Set(15, Number::New(Isolate::GetCurrent(), controllerState.rAxis[2].x));
          buttons->Set(16, Number::New(Isolate::GetCurrent(), controllerState.rAxis[2].y));
          buttons->Set(17, Number::New(Isolate::GetCurrent(), controllerState.rAxis[3].x));
          buttons->Set(18, Number::New(Isolate::GetCurrent(), controllerState.rAxis[3].y));
          buttons->Set(19, Number::New(Isolate::GetCurrent(), controllerState.rAxis[4].x));
          buttons->Set(20, Number::New(Isolate::GetCurrent(), controllerState.rAxis[4].y));

          return info.GetReturnValue().Set(Nan::New<Boolean>(true));
        }
      }
    }
  }

  return info.GetReturnValue().Set(Nan::New<Boolean>(false));
}

NAN_METHOD(IVRSystem::TriggerHapticPulse)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 3)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }
  if (!info[0]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[0] must be a number.");
    return;
  }
  if (!info[1]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[1] must be a number.");
    return;
  }
  if (!info[2]->IsNumber())
  {
    Nan::ThrowTypeError("Argument[2] must be a number.");
    return;
  }
  
  vr::TrackedDeviceIndex_t unControllerDeviceIndex = TO_UINT32(info[0]);
  uint32_t unAxisId = TO_UINT32(info[1]);
  unsigned short usDurationMicroSec  = TO_UINT32(info[2]);

  obj->self_->TriggerHapticPulse(unControllerDeviceIndex, unAxisId, usDurationMicroSec);
}

//=============================================================================
/// virtual bool CaptureInputFocus() = 0;
/* NAN_METHOD(IVRSystem::CaptureInputFocus)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  const auto result = obj->self_->CaptureInputFocus();
  info.GetReturnValue().Set(Nan::New<Boolean>(result));
}

//=============================================================================
/// virtual void ReleaseInputFocus() = 0;
NAN_METHOD(IVRSystem::ReleaseInputFocus)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  obj->self_->ReleaseInputFocus();
}

//=============================================================================
/// virtual bool IsInputFocusCapturedByAnotherProcess() = 0;
NAN_METHOD(IVRSystem::IsInputFocusCapturedByAnotherProcess)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  const auto result = obj->self_->IsInputFocusCapturedByAnotherProcess();
  info.GetReturnValue().Set(Nan::New<Boolean>(result));
} */

//=============================================================================
/// virtual void AcknowledgeQuit_Exiting() = 0;
NAN_METHOD(IVRSystem::AcknowledgeQuit_Exiting)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  obj->self_->AcknowledgeQuit_Exiting();
}

//=============================================================================
/// virtual void AcknowledgeQuit_UserPrompt() = 0;
NAN_METHOD(IVRSystem::AcknowledgeQuit_UserPrompt)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  obj->self_->AcknowledgeQuit_UserPrompt();
}

NAN_METHOD(IVRSystem::GetModelName)
{
  IVRSystem* obj = ObjectWrap::Unwrap<IVRSystem>(info.Holder());

  if (info.Length() != 1)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  uint32_t hand = TO_UINT32(info[0]);
  vr::ETrackedControllerRole role = static_cast<vr::ETrackedControllerRole>(hand + 1);
  vr::TrackedDeviceIndex_t deviceClass = obj->self_->GetTrackedDeviceIndexForControllerRole(role);

  char buf[4096];
  vr::TrackedPropertyError error;
  GetTrackedDeviceIndexForControllerRole(index + 1);
  uint32_t size = GetStringTrackedDeviceProperty(deviceClass, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, buf, sizeof(buf), &error);
  Local<String> modelName = Nan::New<String>(buf, size)->ToLocalChecked();
  info.GetReturnValue().Set(modelName);
}


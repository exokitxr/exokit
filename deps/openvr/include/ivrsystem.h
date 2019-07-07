#ifndef _OPENVR_IVRSYSTEM_H_
#define _OPENVR_IVRSYSTEM_H_

#include <nan.h>
#include <v8.h>
#include <openvr.h>

class IVRSystem : public Nan::ObjectWrap
{
public:
  static NAN_MODULE_INIT(Init);

  // Static factory construction method for other node addons to use.
  static v8::Local<v8::Object> NewInstance(vr::IVRSystem *system);

private:
  explicit IVRSystem(vr::IVRSystem *self);
  ~IVRSystem() = default;

  // Node construction method for new instances.
  static NAN_METHOD(New);

  /// virtual void GetRecommendedRenderTargetSize( uint32_t *pnWidth, uint32_t *pnHeight ) = 0;
  static NAN_METHOD(GetRecommendedRenderTargetSize);

  /// virtual HmdMatrix44_t GetProjectionMatrix( EVREye eEye, float fNearZ, float fFarZ ) = 0;
  static NAN_METHOD(GetProjectionMatrix);

  /// virtual void GetProjectionRaw( EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom ) = 0;
  static NAN_METHOD(GetProjectionRaw);

  /// virtual bool ComputeDistortion( EVREye eEye, float fU, float fV, DistortionCoordinates_t *pDistortionCoordinates ) = 0;
  static NAN_METHOD(ComputeDistortion);

  /// virtual HmdMatrix34_t GetEyeToHeadTransform( EVREye eEye ) = 0;
  static NAN_METHOD(GetEyeToHeadTransform);

  /// virtual bool GetTimeSinceLastVsync( float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter ) = 0;
  static NAN_METHOD(GetTimeSinceLastVsync);

  /// virtual int32_t GetD3D9AdapterIndex() = 0;
  static NAN_METHOD(GetD3D9AdapterIndex);

  /// virtual void GetDXGIOutputInfo( int32_t *pnAdapterIndex ) = 0;
  static NAN_METHOD(GetDXGIOutputInfo);

  /// virtual bool IsDisplayOnDesktop() = 0;
  static NAN_METHOD(IsDisplayOnDesktop);

  /// virtual bool SetDisplayVisibility( bool bIsVisibleOnDesktop ) = 0;
  static NAN_METHOD(SetDisplayVisibility);

  /// virtual void GetDeviceToAbsoluteTrackingPose( ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow, VR_ARRAY_COUNT(unTrackedDevicePoseArrayCount) TrackedDevicePose_t *pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount ) = 0;
  static NAN_METHOD(GetDeviceToAbsoluteTrackingPose);

  /// virtual void ResetSeatedZeroPose() = 0;
  static NAN_METHOD(ResetSeatedZeroPose);

  /// virtual HmdMatrix34_t GetSeatedZeroPoseToStandingAbsoluteTrackingPose() = 0;
  static NAN_METHOD(GetSeatedZeroPoseToStandingAbsoluteTrackingPose);

  /// virtual HmdMatrix34_t GetRawZeroPoseToStandingAbsoluteTrackingPose() = 0;
  static NAN_METHOD(GetRawZeroPoseToStandingAbsoluteTrackingPose);

  /// virtual uint32_t GetSortedTrackedDeviceIndicesOfClass( ETrackedDeviceClass eTrackedDeviceClass, VR_ARRAY_COUNT(unTrackedDeviceIndexArrayCount) vr::TrackedDeviceIndex_t *punTrackedDeviceIndexArray, uint32_t unTrackedDeviceIndexArrayCount, vr::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex = k_unTrackedDeviceIndex_Hmd ) = 0;
  static NAN_METHOD(GetSortedTrackedDeviceIndicesOfClass);

  /// virtual EDeviceActivityLevel GetTrackedDeviceActivityLevel( vr::TrackedDeviceIndex_t unDeviceId ) = 0;
  static NAN_METHOD(GetTrackedDeviceActivityLevel);

  /// virtual void ApplyTransform( TrackedDevicePose_t *pOutputPose, const TrackedDevicePose_t *pTrackedDevicePose, const HmdMatrix34_t *pTransform ) = 0;
  static NAN_METHOD(ApplyTransform);

  /// virtual vr::TrackedDeviceIndex_t GetTrackedDeviceIndexForControllerRole( vr::ETrackedControllerRole unDeviceType ) = 0;
  static NAN_METHOD(GetTrackedDeviceIndexForControllerRole);
  /// virtual vr::ETrackedControllerRole GetControllerRoleForTrackedDeviceIndex( vr::TrackedDeviceIndex_t unDeviceIndex ) = 0;

  /// virtual ETrackedDeviceClass GetTrackedDeviceClass( vr::TrackedDeviceIndex_t unDeviceIndex ) = 0;
  static NAN_METHOD(GetTrackedDeviceClass);
  static NAN_METHOD(GetControllerState);

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
  /// virtual HiddenAreaMesh_t GetHiddenAreaMesh( EVREye eEye, EHiddenAreaMeshType type = k_eHiddenAreaMesh_Standard ) = 0;
  /// virtual bool GetControllerState( vr::TrackedDeviceIndex_t unControllerDeviceIndex, vr::VRControllerState_t *pControllerState, uint32_t unControllerStateSize ) = 0;
  /// virtual bool GetControllerStateWithPose( ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex, vr::VRControllerState_t *pControllerState, uint32_t unControllerStateSize, TrackedDevicePose_t *pTrackedDevicePose ) = 0;
  // virtual void TriggerHapticPulse( vr::TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec ) = 0;
  static NAN_METHOD(TriggerHapticPulse);
  /// virtual const char *GetButtonIdNameFromEnum( EVRButtonId eButtonId ) = 0;
  /// virtual const char *GetControllerAxisTypeNameFromEnum( EVRControllerAxisType eAxisType ) = 0;

  /// virtual bool CaptureInputFocus() = 0;
  static NAN_METHOD(CaptureInputFocus);

  /// virtual void ReleaseInputFocus() = 0;
  static NAN_METHOD(ReleaseInputFocus);

  /// virtual bool IsInputFocusCapturedByAnotherProcess() = 0;
  static NAN_METHOD(IsInputFocusCapturedByAnotherProcess);

  /// virtual uint32_t DriverDebugRequest( vr::TrackedDeviceIndex_t unDeviceIndex, const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) = 0;
  /// virtual vr::EVRFirmwareError PerformFirmwareUpdate( vr::TrackedDeviceIndex_t unDeviceIndex ) = 0;

  /// virtual void AcknowledgeQuit_Exiting() = 0;
  static NAN_METHOD(AcknowledgeQuit_Exiting);

  /// virtual void AcknowledgeQuit_UserPrompt() = 0;
  static NAN_METHOD(AcknowledgeQuit_UserPrompt);

  /// Create a singleton reference to a constructor function.
  static inline Nan::Persistent<v8::Function>& constructor()
  {
    static Nan::Persistent<v8::Function> the_constructor;
    return the_constructor;
  }

  /// Reference to wrapped OpenVR instance.
  vr::IVRSystem * const self_;
  vr::VRInputValueHandle_t handInputSourceHandles[2];
  vr::VRActionHandle_t poseActionHandle;
  vr::VRActionHandle_t handAnimActionHandles[2];

  friend class IVRCompositor;
};

#endif

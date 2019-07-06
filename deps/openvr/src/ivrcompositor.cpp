#include <ivrcompositor.h>

#include <array>
#include <deque>
#include <mutex>
#include <thread>
#include <node.h>
#include <openvr.h>
#include <ivrsystem.h>
#include <functional>
#include <exout>

using namespace v8;

using TrackedDevicePoseArray = std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount>;

namespace vr {
  uv_sem_t reqSem;
  uv_async_t resAsync;
  std::mutex reqMutex;
  std::mutex resMutex;
  std::deque<std::function<void()>> reqCbs;
  std::deque<std::function<void()>> resCbs;
  std::thread reqThead;

  void RunResInMainThread(uv_async_t *handle) {
    Nan::HandleScope scope;

    std::function<void()> resCb;
    {
      std::lock_guard<std::mutex> lock(reqMutex);

      resCb = resCbs.front();
      resCbs.pop_front();
    }
    if (resCb) {
      resCb();
    }
  }
};

VRPoseRes::VRPoseRes(Local<Function> cb) : cb(cb) {}

VRPoseRes::~VRPoseRes() {}

//=============================================================================
NAN_MODULE_INIT(IVRCompositor::Init)
{
  // Create a function template that is called in JS to create this wrapper.
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);

  // Declare human-readable name for this wrapper.
  tpl->SetClassName(Nan::New("IVRCompositor").ToLocalChecked());

  // Declare the stored number of fields (just the wrapped C++ object).
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Assign all the wrapped methods of this object.
  // Nan::SetPrototypeMethod(tpl, "WaitGetPoses", WaitGetPoses);
  Nan::SetPrototypeMethod(tpl, "RequestGetPoses", RequestGetPoses);
  Nan::SetPrototypeMethod(tpl, "Submit", Submit);

  // Set a static constructor function to reference the `New` function template.
  constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());

  uv_sem_init(&vr::reqSem, 0);
  uv_loop_t *loop = windowsystembase::GetEventLoop();
  uv_async_init(loop, &vr::resAsync, vr::RunResInMainThread);
  vr::reqThead = std::thread([]() -> void {
    for (;;) {
      uv_sem_wait(&vr::reqSem);

      std::function<void()> reqCb;
      {
        std::lock_guard<std::mutex> lock(vr::reqMutex);

        if (vr::reqCbs.size() > 0) {
          reqCb = vr::reqCbs.front();
          vr::reqCbs.pop_front();
        }
      }
      if (reqCb) {
        reqCb();
      } else {
        break;
      }
    }
  });
}

//=============================================================================
Local<Object> IVRCompositor::NewInstance(vr::IVRCompositor *compositor)
{
  Nan::EscapableHandleScope scope;
  Local<Function> cons = Nan::New(constructor());
  Local<Value> argv[1] = { Nan::New<External>(compositor) };
  return scope.Escape(Nan::NewInstance(cons, 1, argv).ToLocalChecked());
}

//=============================================================================
IVRCompositor::IVRCompositor(vr::IVRCompositor *self)
: self_(self)
{
  {
    vr::EVRInputError error = vr::VRInput()->GetActionHandle("/actions/main/in/LeftHand", &leftHandActionHandle);
    if (error != vr::EVRInputError::VRInputError_None) {
      Nan::ThrowError("Failed to get left hand action handle");
    }
  }
  {
    vr::EVRInputError error = vr::VRInput()->GetActionHandle("/actions/main/in/LeftHand_Anim", &leftHandAnimActionHandle);
    if (error != vr::EVRInputError::VRInputError_None) {
      Nan::ThrowError("Failed to get left hand anim action handle");
    }
  }
  {
    vr::EVRInputError error = vr::VRInput()->GetActionHandle("/actions/main/in/RightHand", &rightHandActionHandle);
    if (error != vr::EVRInputError::VRInputError_None) {
      Nan::ThrowError("Failed to get right hand action handle");
    }
  }
  {
    vr::EVRInputError error = vr::VRInput()->GetActionHandle("/actions/main/in/RightHand_Anim", &rightHandAnimActionHandle);
    if (error != vr::EVRInputError::VRInputError_None) {
      Nan::ThrowError("Failed to get right hand anim action handle");
    }
  }
  {
    vr::EVRInputError error = vr::VRInput()->GetActionSetHandle("/actions/main", &actionSetHandle);
    if (error != vr::EVRInputError::VRInputError_None) {
      Nan::ThrowError("Failed to get main action set handle");
    }
  }
}

//=============================================================================
NAN_METHOD(IVRCompositor::New)
{
  if (!info.IsConstructCall())
  {
    Nan::ThrowError("Use the `new` keyword when creating a new instance.");
    return;
  }

  if (info.Length() != 1 || !info[0]->IsExternal())
  {
    Nan::ThrowTypeError("Argument[0] must be an `IVRCompositor*`.");
    return;
  }

  auto wrapped_instance = static_cast<vr::IVRCompositor*>(
    Local<External>::Cast(info[0])->Value());
  IVRCompositor *obj = new IVRCompositor(wrapped_instance);
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

/* NAN_METHOD(IVRCompositor::WaitGetPoses)
{
  IVRCompositor* obj = ObjectWrap::Unwrap<IVRCompositor>(info.Holder());

  if (info.Length() != 4)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  TrackedDevicePoseArray trackedDevicePoseArray;
	obj->self_->WaitGetPoses(trackedDevicePoseArray.data(), static_cast<uint32_t>(trackedDevicePoseArray.size()), nullptr, 0);

  IVRSystem* system = IVRSystem::Unwrap<IVRSystem>(Local<Object>::Cast(info[0]));
  Local<Float32Array> hmdFloat32Array = Local<Float32Array>::Cast(info[1]);
  Local<Float32Array> leftControllerFloat32Array = Local<Float32Array>::Cast(info[2]);
  Local<Float32Array> rightControllerFloat32Array = Local<Float32Array>::Cast(info[3]);
  float *hmdArray = (float *)((char *)hmdFloat32Array->Buffer()->GetContents().Data() + hmdFloat32Array->ByteOffset());
  float *leftControllerArray = (float *)((char *)leftControllerFloat32Array->Buffer()->GetContents().Data() + leftControllerFloat32Array->ByteOffset());
  float *rightControllerArray = (float *)((char *)rightControllerFloat32Array->Buffer()->GetContents().Data() + rightControllerFloat32Array->ByteOffset());

  memset(hmdArray, std::numeric_limits<float>::quiet_NaN(), 16);
  memset(leftControllerArray, std::numeric_limits<float>::quiet_NaN(), 16);
  memset(rightControllerArray, std::numeric_limits<float>::quiet_NaN(), 16);

  for (unsigned int i = 0; i < trackedDevicePoseArray.size(); i++) {
    const vr::TrackedDevicePose_t &trackedDevicePose = trackedDevicePoseArray[i];
    if (trackedDevicePose.bPoseIsValid) {
      const vr::ETrackedDeviceClass deviceClass = system->self_->GetTrackedDeviceClass(i);
      if (deviceClass == vr::TrackedDeviceClass_HMD) {
        const vr::HmdMatrix34_t &matrix = trackedDevicePose.mDeviceToAbsoluteTracking;

        for (unsigned int v = 0; v < 4; v++) {
          for (unsigned int u = 0; u < 3; u++) {
            hmdFloat32Array->Set(v * 4 + u, Number::New(Isolate::GetCurrent(), matrix.m[u][v]));
          }
        }
        hmdArray[0 * 4 + 3] = 0;
        hmdArray[1 * 4 + 3] = 0;
        hmdArray[2 * 4 + 3] = 0;
        hmdArray[3 * 4 + 3] = 1;
      } else if (deviceClass == vr::TrackedDeviceClass_Controller) {
        const vr::ETrackedControllerRole controllerRole = system->self_->GetControllerRoleForTrackedDeviceIndex(i);
        if (controllerRole == vr::TrackedControllerRole_LeftHand) {
          const vr::HmdMatrix34_t &matrix = trackedDevicePose.mDeviceToAbsoluteTracking;

          for (unsigned int v = 0; v < 4; v++) {
            for (unsigned int u = 0; u < 3; u++) {
              leftControllerArray[v * 4 + u] = matrix.m[u][v];
            }
          }
          leftControllerArray[0 * 4 + 3] = 0;
          leftControllerArray[1 * 4 + 3] = 0;
          leftControllerArray[2 * 4 + 3] = 0;
          leftControllerArray[3 * 4 + 3] = 1;
        } else if (controllerRole == vr::TrackedControllerRole_RightHand) {
          const vr::HmdMatrix34_t &matrix = trackedDevicePose.mDeviceToAbsoluteTracking;

          for (unsigned int v = 0; v < 4; v++) {
            for (unsigned int u = 0; u < 3; u++) {
              rightControllerArray[v * 4 + u] = matrix.m[u][v];
            }
          }
          rightControllerArray[0 * 4 + 3] = 0;
          rightControllerArray[1 * 4 + 3] = 0;
          rightControllerArray[2 * 4 + 3] = 0;
          rightControllerArray[3 * 4 + 3] = 1;
        }
      }
    }
  }
} */

void setPoseMatrix(float *dstMatrixArray, const vr::HmdMatrix34_t &srcMatrix) {
  for (unsigned int v = 0; v < 4; v++) {
    for (unsigned int u = 0; u < 3; u++) {
      dstMatrixArray[v * 4 + u] = srcMatrix.m[u][v];
    }
  }
  dstMatrixArray[0 * 4 + 3] = 0;
  dstMatrixArray[1 * 4 + 3] = 0;
  dstMatrixArray[2 * 4 + 3] = 0;
  dstMatrixArray[3 * 4 + 3] = 1;
}
NAN_METHOD(IVRCompositor::RequestGetPoses) {
  if (info.Length() != 3)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  IVRCompositor* obj = ObjectWrap::Unwrap<IVRCompositor>(info.Holder());
  IVRSystem* system = IVRSystem::Unwrap<IVRSystem>(Local<Object>::Cast(info[0]));
  Local<Float32Array> posesFloat32Array = Local<Float32Array>::Cast(info[1]);
  Local<Function> cbFn = Local<Function>::Cast(info[2]);

  float *hmdArray = (float *)((char *)posesFloat32Array->Buffer()->GetContents().Data() + posesFloat32Array->ByteOffset());
  float *leftControllerArray = hmdArray + 1*16;
  float *rightControllerArray = hmdArray + 2*16;
  float *trackerArraysStart = hmdArray + 3*16;
  VRPoseRes *vrPoseRes = new VRPoseRes(cbFn);

  {
    std::lock_guard<std::mutex> lock(vr::reqMutex);

    vr::reqCbs.push_back([obj, system, hmdArray, leftControllerArray, rightControllerArray, trackerArraysStart, vrPoseRes]() -> void {
      TrackedDevicePoseArray trackedDevicePoseArray;
	    obj->self_->WaitGetPoses(trackedDevicePoseArray.data(), static_cast<uint32_t>(trackedDevicePoseArray.size()), nullptr, 0);

      vr::VRActiveActionSet_t activeActionSet;
      activeActionSet.ulActionSet = obj->actionSetHandle;
      activeActionSet.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
      activeActionSet.nPriority = 0;
      vr::VRInput()->UpdateActionState(&activeActionSet, sizeof(activeActionSet), 1);

      const float identityMatrix[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
      memcpy(hmdArray, identityMatrix, sizeof(identityMatrix));
      memcpy(leftControllerArray, identityMatrix, sizeof(identityMatrix));
      memcpy(rightControllerArray, identityMatrix, sizeof(identityMatrix));

      {
        vr::VRSkeletalSummaryData_t skeletalSummaryData;
        vr::EVRInputError error = GetSkeletalSummaryData(obj->leftHandAnimActionHandle, &skeletalSummaryData);
        if (error) {
          exerr << "failed to get left hand anim skeletal summary data" << std::endl;
        }
        exout << "left finger curls:";
        for (int i = 0; i < VRFinger_Count; i++) {
          exout << skeletalSummaryData.flFingerCurl[i] << (i != (VRFinger_Count-1) ? "," : "");
        }
        exout << std::endl;

        vr::InputSkeletalActionData_t skeletalActionData;
        vr::VRInput()->GetSkeletalActionData(obj->leftHandAnimActionHandle, &skeletalActionData, sizeof(skeletalActionData));
        if (error) {
          exerr << "failed to get left hand anim skeletal action data" << std::endl;
        }
        if (skeletalActionData.bActive) {
          uint32_t boneCount = 0;
          error = vr::VRInput()->GetBoneCount(obj->leftHandAnimActionHandle, &boneCount);
          if (error) {
            exerr << "failed to get left hand anim bone count" << std::endl;
          }

          std::vector<vr::VRBoneTransform_t> bones(boneCount);
          error = vr::VRInput()->GetSkeletalBoneData(obj->leftHandAnimActionHandle, vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model, vr::EVRSkeletalMotionRange::VRSkeletalMotionRange_WithController, bones.data(), boneCount);
          if (error) {
            exerr << "failed to get left hand anim skeletal bone data" << std::endl;
          }

          exout << "got left bones " << boneCount << std::endl; // XXX
        }
      }
      {
        vr::VRSkeletalSummaryData_t skeletalSummaryData;
        vr::EVRInputError error = GetSkeletalSummaryData(obj->rightHandAnimActionHandle, &skeletalSummaryData);
        if (error) {
          exerr << "failed to get right hand anim skeletal summary data" << std::endl;
        }
        exout << "right finger curls:";
        for (int i = 0; i < VRFinger_Count; i++) {
          exout << skeletalSummaryData.flFingerCurl[i] << (i != (VRFinger_Count-1) ? "," : "");
        }
        exout << std::endl;

        vr::InputSkeletalActionData_t skeletalActionData;
        error = vr::VRInput()->GetSkeletalActionData(obj->rightHandAnimActionHandle, &skeletalActionData, sizeof(skeletalActionData));
        if (error) {
          exerr << "failed to get right hand anim skeletal action data" << std::endl;
        }
        if (skeletalActionData.bActive) {
          uint32_t boneCount = 0;
          error = vr::VRInput()->GetBoneCount(obj->rightHandAnimActionHandle, &boneCount);
          if (error) {
            exerr << "failed to get right hand anim bone count" << std::endl;
          }

          std::vector<vr::VRBoneTransform_t> bones(boneCount);
          error = vr::VRInput()->GetSkeletalBoneData(obj->rightHandAnimActionHandle, vr::EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model, vr::EVRSkeletalMotionRange::VRSkeletalMotionRange_WithController, bones.data(), boneCount);
          if (error) {
            exerr << "failed to get right hand anim skeletal bone data" << std::endl;
          }

          exout << "got right bones " << boneCount << std::endl; // XXX
        }
      }

      unsigned int numTrackers = 0;
      const unsigned int maxNumTrackers = 8;

      for (unsigned int i = 0; i < trackedDevicePoseArray.size(); i++) {
        const vr::TrackedDevicePose_t &trackedDevicePose = trackedDevicePoseArray[i];

        if (trackedDevicePose.bPoseIsValid) {
          const vr::ETrackedDeviceClass deviceClass = system->self_->GetTrackedDeviceClass(i);

          if (deviceClass == vr::TrackedDeviceClass_HMD) {
            setPoseMatrix(hmdArray, trackedDevicePose.mDeviceToAbsoluteTracking);
          } else if (deviceClass == vr::TrackedDeviceClass_Controller) {
            const vr::ETrackedControllerRole controllerRole = system->self_->GetControllerRoleForTrackedDeviceIndex(i);

            if (controllerRole == vr::TrackedControllerRole_LeftHand) {
              setPoseMatrix(leftControllerArray, trackedDevicePose.mDeviceToAbsoluteTracking);
            } else if (controllerRole == vr::TrackedControllerRole_RightHand) {
              setPoseMatrix(rightControllerArray, trackedDevicePose.mDeviceToAbsoluteTracking);
            }
          } else if (deviceClass == vr::TrackedDeviceClass_GenericTracker) {
            if (numTrackers < maxNumTrackers) {
              float *trackerArray = trackerArraysStart + numTrackers*16;
              setPoseMatrix(trackerArray, trackedDevicePose.mDeviceToAbsoluteTracking);
              numTrackers++;
            }
          }
        }
      }
      for (unsigned int i = numTrackers; i < maxNumTrackers; i++) {
        float *trackerArray = trackerArraysStart + i*16;
        trackerArray[0] = std::numeric_limits<float>::quiet_NaN();
      }

      {
        std::lock_guard<std::mutex> lock(vr::resMutex);

        vr::resCbs.push_back([vrPoseRes]() -> void {
          {
            Local<Object> asyncObject = Nan::New<Object>();
            AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "IVRCompositor::RequestGetPoses");

            Local<Function> cb = Nan::New(vrPoseRes->cb);
            asyncResource.MakeCallback(cb, 0, nullptr);
          }

          delete vrPoseRes;
        });
      }

      uv_async_send(&vr::resAsync);
    });
  }

  uv_sem_post(&vr::reqSem);
}

NAN_METHOD(IVRCompositor::Submit)
{
  IVRCompositor* obj = ObjectWrap::Unwrap<IVRCompositor>(info.Holder());

  if (info.Length() != 1)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  if (!info[0]->IsNumber())
  {
    Nan::ThrowError("Expected arguments (number).");
    return;
  }

  GLuint tex = TO_UINT32(info[0]);

  vr::EColorSpace colorSpace = vr::ColorSpace_Gamma;

  vr::Texture_t leftEyeTexture = {(void *)(size_t)tex, vr::TextureType_OpenGL, colorSpace};
  vr::VRTextureBounds_t leftEyeTextureBounds = {
    0, 0,
    0.5, 1,
  };
  vr::EVRCompositorError compositorError = obj->self_->Submit(vr::Eye_Left, &leftEyeTexture, &leftEyeTextureBounds);
  if (compositorError != vr::VRCompositorError_None) {
    if (compositorError == vr::VRCompositorError_RequestFailed) Nan::ThrowError("Compositor error: VRCompositorError_RequestFailed");
    else if (compositorError == vr::VRCompositorError_IncompatibleVersion) Nan::ThrowError("Compositor error: VRCompositorError_IncompatibleVersion");
    else if (compositorError == vr::VRCompositorError_DoNotHaveFocus) {} // Nan::ThrowError("Compositor error: VRCompositorError_DoNotHaveFocus");
    else if (compositorError == vr::VRCompositorError_InvalidTexture) Nan::ThrowError("Compositor error: VRCompositorError_InvalidTexture");
    else if (compositorError == vr::VRCompositorError_IsNotSceneApplication) Nan::ThrowError("Compositor error: VRCompositorError_IsNotSceneApplication");
    else if (compositorError == vr::VRCompositorError_TextureIsOnWrongDevice) Nan::ThrowError("Compositor error: VRCompositorError_TextureIsOnWrongDevice");
    else if (compositorError == vr::VRCompositorError_TextureUsesUnsupportedFormat) Nan::ThrowError("Compositor error: VRCompositorError_TextureUsesUnsupportedFormat");
    else if (compositorError == vr::VRCompositorError_SharedTexturesNotSupported) Nan::ThrowError("Compositor error: VRCompositorError_SharedTexturesNotSupported");
    else if (compositorError == vr::VRCompositorError_IndexOutOfRange) Nan::ThrowError("Compositor error: VRCompositorError_IndexOutOfRange");
    else if (compositorError == vr::VRCompositorError_AlreadySubmitted) Nan::ThrowError("Compositor error: VRCompositorError_AlreadySubmitted");
    else if (compositorError == vr::VRCompositorError_InvalidBounds) Nan::ThrowError("Compositor error: VRCompositorError_InvalidBounds");
    else Nan::ThrowError("Compositor error: unknown");
    return;
  }

  vr::Texture_t rightEyeTexture = {(void *)(size_t)tex, vr::TextureType_OpenGL, colorSpace};
  vr::VRTextureBounds_t rightEyeTextureBounds = {
    0.5, 0,
    1, 1,
  };
  compositorError = obj->self_->Submit(vr::Eye_Right, &rightEyeTexture, &rightEyeTextureBounds);
  if (compositorError != vr::VRCompositorError_None) {
    if (compositorError == vr::VRCompositorError_RequestFailed) Nan::ThrowError("Compositor error: VRCompositorError_RequestFailed");
    else if (compositorError == vr::VRCompositorError_IncompatibleVersion) Nan::ThrowError("Compositor error: VRCompositorError_IncompatibleVersion");
    else if (compositorError == vr::VRCompositorError_DoNotHaveFocus) {} // Nan::ThrowError("Compositor error: VRCompositorError_DoNotHaveFocus");
    else if (compositorError == vr::VRCompositorError_InvalidTexture) Nan::ThrowError("Compositor error: VRCompositorError_InvalidTexture");
    else if (compositorError == vr::VRCompositorError_IsNotSceneApplication) Nan::ThrowError("Compositor error: VRCompositorError_IsNotSceneApplication");
    else if (compositorError == vr::VRCompositorError_TextureIsOnWrongDevice) Nan::ThrowError("Compositor error: VRCompositorError_TextureIsOnWrongDevice");
    else if (compositorError == vr::VRCompositorError_TextureUsesUnsupportedFormat) Nan::ThrowError("Compositor error: VRCompositorError_TextureUsesUnsupportedFormat");
    else if (compositorError == vr::VRCompositorError_SharedTexturesNotSupported) Nan::ThrowError("Compositor error: VRCompositorError_SharedTexturesNotSupported");
    else if (compositorError == vr::VRCompositorError_IndexOutOfRange) Nan::ThrowError("Compositor error: VRCompositorError_IndexOutOfRange");
    else if (compositorError == vr::VRCompositorError_AlreadySubmitted) Nan::ThrowError("Compositor error: VRCompositorError_AlreadySubmitted");
    else if (compositorError == vr::VRCompositorError_InvalidBounds) Nan::ThrowError("Compositor error: VRCompositorError_InvalidBounds");
    else Nan::ThrowError("Compositor error: unknown");
    return;
  }

  obj->self_->PostPresentHandoff();
}

NAN_METHOD(NewCompositor) {
  if (info.Length() != 0)
  {
    Nan::ThrowError("Wrong number of arguments.");
    return;
  }

  // Perform the actual wrapped call.
  vr::IVRCompositor *compositor = vr::VRCompositor();
  if (!compositor)
  {
    Nan::ThrowError("Unable to initialize VR compositor.");
    return;
  }

  // Wrap the resulting system in the correct wrapper and return it.
  auto result = IVRCompositor::NewInstance(compositor);
  info.GetReturnValue().Set(result);
}

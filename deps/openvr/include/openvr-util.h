#ifndef _OPENVR_UTIL_H_
#define _OPENVR_UTIL_H_

#include <algorithm>
#include <limits>
#include <nan.h>
#include <openvr.h>
#include <v8.h>

//=============================================================================
/// Template conversion function from C++ to Node.
template<typename T>
v8::Local<v8::Value> encode(const T &value);

/// Template conversion function from Node to C++.
template<typename T>
T decode(const v8::Local<v8::Value>);

//=============================================================================
template<>
v8::Local<v8::Value> encode(const vr::HmdMatrix44_t &value)
{
  Nan::EscapableHandleScope scope;
  auto result = Nan::New<v8::Array>();

  for (uint32_t rowIdx = 0; rowIdx < 4; ++rowIdx)
  {
    auto row = Nan::New<v8::Array>();
    for (uint32_t colIdx = 0; colIdx < 4; ++colIdx)
    {
      Nan::Set(row, colIdx, Nan::New<v8::Number>(value.m[rowIdx][colIdx]));
    }
    Nan::Set(result, rowIdx, row);
  }

  return scope.Escape(result);
}

//=============================================================================
template<>
v8::Local<v8::Value> encode(const vr::HmdMatrix34_t &value)
{
  Nan::EscapableHandleScope scope;
  auto result = Nan::New<v8::Array>();

  for (uint32_t rowIdx = 0; rowIdx < 3; ++rowIdx)
  {
    auto row = Nan::New<v8::Array>();
    for (uint32_t colIdx = 0; colIdx < 4; ++colIdx)
    {
      Nan::Set(row, colIdx, Nan::New<v8::Number>(value.m[rowIdx][colIdx]));
    }
    Nan::Set(result, rowIdx, row);
  }

  return scope.Escape(result);
}

//=============================================================================
template<>
vr::HmdMatrix34_t decode(const v8::Local<v8::Value> value)
{
  vr::HmdMatrix34_t result;
  const auto matrix = value->ToObject();

  for (uint32_t rowIdx = 0; rowIdx < 3; ++rowIdx)
  {
    const auto row = Nan::Get(matrix, rowIdx).ToLocalChecked()->ToObject();
    for (uint32_t colIdx = 0; colIdx < 4; ++colIdx)
    {
      const auto cell = Nan::Get(row, colIdx).ToLocalChecked();
      result.m[rowIdx][colIdx] = static_cast<float>(cell->NumberValue());
    }
  }

  return result;
}

//=============================================================================
template<>
v8::Local<v8::Value> encode(const vr::HmdVector3_t &value)
{
  Nan::EscapableHandleScope scope;
  auto result = Nan::New<v8::Array>();

  for (uint32_t idx = 0; idx < 3; ++idx)
  {
    Nan::Set(result, idx, Nan::New<v8::Number>(value.v[idx]));
  }

  return scope.Escape(result);
}

//=============================================================================
template<>
vr::HmdVector3_t decode(const v8::Local<v8::Value> value)
{
  vr::HmdVector3_t result;
  const auto array = value->ToObject();

  for (uint32_t idx = 0; idx < 3; ++idx)
  {
    const auto cell = Nan::Get(array, idx).ToLocalChecked();
    result.v[idx] = static_cast<float>(cell->NumberValue());
  }

  return result;
}

//=============================================================================
template<>
v8::Local<v8::Value> encode(const vr::DistortionCoordinates_t &value)
{
  Nan::EscapableHandleScope scope;
  auto result = Nan::New<v8::Object>();

  // Extract red values.
  {
    auto red = Nan::New<v8::Array>();
    Nan::Set(red, 0, Nan::New<v8::Number>(value.rfRed[0]));
    Nan::Set(red, 1, Nan::New<v8::Number>(value.rfRed[1]));

    auto red_prop = Nan::New<v8::String>("red").ToLocalChecked();
    Nan::Set(result, red_prop, red);
  }

  // Extract green values.
  {
    auto green = Nan::New<v8::Array>();
    Nan::Set(green, 0, Nan::New<v8::Number>(value.rfGreen[0]));
    Nan::Set(green, 1, Nan::New<v8::Number>(value.rfGreen[1]));

    auto green_prop = Nan::New<v8::String>("green").ToLocalChecked();
    Nan::Set(result, green_prop, green);
  }

  // Extract blue values.
  {
    auto blue = Nan::New<v8::Array>();
    Nan::Set(blue, 0, Nan::New<v8::Number>(value.rfBlue[0]));
    Nan::Set(blue, 1, Nan::New<v8::Number>(value.rfBlue[1]));

    auto blue_prop = Nan::New<v8::String>("blue").ToLocalChecked();
    Nan::Set(result, blue_prop, blue);
  }

  return scope.Escape(result);
}

//=============================================================================
template<>
v8::Local<v8::Value> encode(const vr::TrackedDevicePose_t &value)
{
  Nan::EscapableHandleScope scope;
  auto result = Nan::New<v8::Object>();

  auto deviceToAbsoluteTracking_prop =
    Nan::New<v8::String>("deviceToAbsoluteTracking").ToLocalChecked();
  Nan::Set(
    result, deviceToAbsoluteTracking_prop,
    encode(value.mDeviceToAbsoluteTracking)
  );

  auto velocity_prop =
    Nan::New<v8::String>("velocity").ToLocalChecked();
  Nan::Set(result, velocity_prop, encode(value.vVelocity));

  auto angularVelocity_prop =
    Nan::New<v8::String>("angularVelocity").ToLocalChecked();
  Nan::Set(result, angularVelocity_prop, encode(value.vAngularVelocity));

  auto trackingResult_prop =
    Nan::New<v8::String>("trackingResult").ToLocalChecked();
  Nan::Set(
    result, trackingResult_prop,
    Nan::New<v8::Number>(static_cast<uint32_t>(value.eTrackingResult))
  );

  auto poseIsValid_prop =
    Nan::New<v8::String>("poseIsValid").ToLocalChecked();
  Nan::Set(
    result, poseIsValid_prop,
    Nan::New<v8::Boolean>(value.bPoseIsValid)
  );

  auto deviceIsConnected_prop =
    Nan::New<v8::String>("deviceIsConnected").ToLocalChecked();
  Nan::Set(
    result, deviceIsConnected_prop,
    Nan::New<v8::Boolean>(value.bDeviceIsConnected)
  );

  return scope.Escape(result);
}

//=============================================================================
template<>
vr::TrackedDevicePose_t decode(const v8::Local<v8::Value> value)
{
  vr::TrackedDevicePose_t result;
  const auto object = value->ToObject();

  auto deviceToAbsoluteTracking_prop =
    Nan::New<v8::String>("deviceToAbsoluteTracking").ToLocalChecked();
  result.mDeviceToAbsoluteTracking =
    decode<vr::HmdMatrix34_t>(
      Nan::Get(object, deviceToAbsoluteTracking_prop).ToLocalChecked());

  auto velocity_prop =
    Nan::New<v8::String>("velocity").ToLocalChecked();
  result.vVelocity =
    decode<vr::HmdVector3_t>(Nan::Get(object, velocity_prop).ToLocalChecked());

  auto angularVelocity_prop =
    Nan::New<v8::String>("angularVelocity").ToLocalChecked();
  result.vAngularVelocity =
    decode<vr::HmdVector3_t>(
      Nan::Get(object, angularVelocity_prop).ToLocalChecked());

  auto trackingResult_prop =
    Nan::New<v8::String>("trackingResult").ToLocalChecked();
  result.eTrackingResult =
    static_cast<vr::ETrackingResult>(
      Nan::Get(object, trackingResult_prop).ToLocalChecked()->Uint32Value());

  auto poseIsValid_prop =
    Nan::New<v8::String>("poseIsValid").ToLocalChecked();
  result.bPoseIsValid =
    Nan::Get(object, poseIsValid_prop).ToLocalChecked()->BooleanValue();

  auto deviceIsConnected_prop =
    Nan::New<v8::String>("deviceIsConnected").ToLocalChecked();
  result.bDeviceIsConnected =
    Nan::Get(object, deviceIsConnected_prop).ToLocalChecked()->BooleanValue();

  return result;
}

//=============================================================================
template<uint32_t N>
v8::Local<v8::Value> encode(const std::array<vr::TrackedDevicePose_t, N> &value)
{
  Nan::EscapableHandleScope scope;
  auto result = Nan::New<v8::Array>();

  for (uint32_t idx = 0; idx < N; ++idx)
    Nan::Set(result, idx, encode(value[idx]));

  return scope.Escape(result);
}

//=============================================================================
/// Encode either all elements or up to specified range.
template<uint32_t N>
v8::Local<v8::Value> encode(
  const std::array<uint32_t, N> &value,
  uint32_t numElementsToencode = std::numeric_limits<uint32_t>::max())
{
  Nan::EscapableHandleScope scope;
  auto result = Nan::New<v8::Array>();

  const uint32_t numElements = std::min(N, numElementsToencode);
  for (uint32_t idx = 0; idx < numElements; ++idx)
    Nan::Set(result, idx, Nan::New<v8::Number>(value[idx]));

  return scope.Escape(result);
}

#endif

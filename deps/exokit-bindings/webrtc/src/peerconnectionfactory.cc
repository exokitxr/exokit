/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "peerconnectionfactory.h"

#include "webrtc/rtc_base/ssladapter.h"
#include "webrtc/api/video_codecs/video_encoder_factory.h"
#include "webrtc/api/video_codecs/video_decoder_factory.h"

#include "common.h"
#include "../../helpers.h"

using node_webrtc::PeerConnectionFactory;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Integer;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Uint32;
using v8::Value;
using v8::Array;

Nan::Persistent<Function> PeerConnectionFactory::constructor;
std::shared_ptr<PeerConnectionFactory> PeerConnectionFactory::_default;
uv_mutex_t PeerConnectionFactory::_lock;
int PeerConnectionFactory::_references = 0;

PeerConnectionFactory::PeerConnectionFactory(rtc::scoped_refptr<webrtc::AudioDeviceModule> audioDeviceModule) {
  TRACE_CALL;
  
  _signalingThread = std::unique_ptr<rtc::Thread>(new rtc::Thread());
  assert(_signalingThread);

  {
    bool result = _signalingThread->Start();
    assert(result);
  }

  _factory = webrtc::CreatePeerConnectionFactory(nullptr, nullptr, _signalingThread.get(), audioDeviceModule, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  assert(_factory);

  TRACE_END;
}

PeerConnectionFactory::~PeerConnectionFactory() {
  TRACE_CALL;

  _factory = nullptr;

  _signalingThread->Stop();
  _signalingThread = nullptr;

  TRACE_END;
}

NAN_METHOD(PeerConnectionFactory::New) {
  TRACE_CALL;
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct a PeerConnectionFactory.");
  }

  // TODO(mroberts): Read `audioLayer` from some PeerConnectionFactoryOptions?
  auto peerConnectionFactory = new PeerConnectionFactory();
  peerConnectionFactory->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

std::shared_ptr<PeerConnectionFactory> PeerConnectionFactory::GetOrCreateDefault() {
  uv_mutex_lock(&_lock);
  _references++;
  if (_references == 1) {
    _default = std::make_shared<PeerConnectionFactory>();
  }
  uv_mutex_unlock(&_lock);
  return _default;
}

void PeerConnectionFactory::Release() {
  uv_mutex_lock(&_lock);
  _references--;
  assert(_references >= 0);
  if (!_references) {
    _default = nullptr;
  }
  uv_mutex_unlock(&_lock);
}

void PeerConnectionFactory::Dispose() {
  uv_mutex_destroy(&_lock);
  rtc::CleanupSSL();
}

void PeerConnectionFactory::Init(v8::Local<Object> exports) {
  uv_mutex_init(&_lock);

  bool result;
  result = rtc::InitializeSSL();
  assert(result);

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PeerConnectionFactory").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor.Reset(JS_FUNC(tpl));
  exports->Set(Nan::New("PeerConnectionFactory").ToLocalChecked(), JS_FUNC(tpl));
}

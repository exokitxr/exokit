/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "node.h"

#include "webrtc.h"
#include "datachannel.h"
#include "rtcstatsreport.h"
#include "rtcstatsresponse.h"
#include "peerconnection.h"
#include "peerconnectionfactory.h"

using namespace v8;

namespace node_webrtc {

/* void dispose(void*) {
  node_webrtc::PeerConnectionFactory::Dispose();
} */

void init(v8::Local<v8::Object> exports) {
  node_webrtc::PeerConnectionFactory::Init(exports);
  node_webrtc::PeerConnection::Init(exports);
  node_webrtc::DataChannel::Init(exports);
  node_webrtc::RTCStatsReport::Init(exports);
  node_webrtc::RTCStatsResponse::Init(exports);
  // node::AtExit(dispose);
}

}

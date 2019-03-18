'use strict';

class RTCTrackEvent {
  constructor(eventInit = {}) {
    this._receiver = eventInit.receiver;
    this._streams = eventInit.streams;
    this._track = eventInit.track;
    this._transciever = eventInit.transciever;
  }
  get receiver() { return this._receiver; }
  set receiver(receiver) {}
  get streams() { return this._streams; }
  set streams(streams) {}
  get track() { return this._track; }
  set track(track) {}
  get transceiver() { return this._transceiver; }
  set transceiver(transceiver) {}
}

module.exports = RTCTrackEvent;

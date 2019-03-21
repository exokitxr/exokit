'use strict';

var EventTarget = require('./eventtarget');

var RTCDataChannelMessageEvent = require('./datachannelmessageevent');

function RTCDataChannel(internalDC) {
  var self = this;

  EventTarget.call(this);

  internalDC.onerror = function onerror() {
    self.dispatchEvent({ type: 'error' });
  };

  internalDC.onmessage = function onmessage(data) {
    self.dispatchEvent(new RTCDataChannelMessageEvent(data));
  };

  internalDC.onstatechange = function onstatechange(data) {
    switch (data) {
      case 'open':
        self.dispatchEvent({ type: 'open' });
        break;

      case 'closed':
        self.dispatchEvent({ type: 'close' });
        break;
    }
  };

  Object.defineProperties(this, {
    bufferedAmount: {
      get: function getBufferedAmount() {
        return internalDC.bufferedAmount;
      },
      configurable: true,
    },
    id: {
      get: function getId() {
        return internalDC.id;
      },
      configurable: true,
    },
    label: {
      get: function getLabel() {
        return internalDC.label;
      },
      configurable: true,
    },
    maxRetransmits: {
      get: function getMaxRetransmits() {
        return internalDC.maxRetransmits;
      },
      configurable: true,
    },
    ordered: {
      get: function getOrdered() {
        return internalDC.ordered;
      },
      configurable: true,
    },
    priority: {
      get: function getPriority() {
        return internalDC.priority;
      },
      configurable: true,
    },
    protocol: {
      get: function getProtocol() {
        return internalDC.protocol;
      },
      configurable: true,
    },
    readyState: {
      get: function getReadyState() {
        return internalDC.readyState;
      },
      configurable: true,
    },
    binaryType: {
      get: function getBinaryType() {
        return internalDC.binaryType;
      },
      set: function(binaryType) {
        internalDC.binaryType = binaryType;
      },
      configurable: true,
    }
  });

  this.send = function send(data) {
    // NOTE(mroberts): Here's a hack to support jsdom's Blob implementation.
    var implSymbol = Object.getOwnPropertySymbols(data).find(function(symbol) {
      return symbol.toString() === 'Symbol(impl)';
    });
    if (data[implSymbol] && data[implSymbol]._buffer) {
      data = data[implSymbol]._buffer;
    }

    internalDC.send(data);
  };

  this.close = function close() {
    internalDC.close();
  };
}

module.exports = RTCDataChannel;

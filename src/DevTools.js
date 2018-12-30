const url = require('url');
const http = require('http');
const htermRepl = require('hterm-repl');

const DOM = require('./DOM');
const {HTMLIframeElement} = DOM;

// const DEVTOOLS_PORT = 9223;

const _getReplServer = (() => {
  let replServer = null;
  return () => new Promise((accept, reject) => {
    if (!replServer) {
      htermRepl(null, (err, newReplServer) => {
        if (!err) {
          replServer = newReplServer;
          accept(replServer);
        } else  {
          reject(err);
        }
      });
    } else {
      accept(replServer);
    }
  });
})();

let id = 0;
class DevTools {
  constructor(context, document, replServer) {
    this.context = context;
    this.document = document;
    this.replServer = replServer;
    this.id = (++id) + '';
    this.repls = [];
    this.iframe = (() => {
      const iframe = document.createElement('iframe');
      iframe.d = 2;
      iframe.onload = () => {
        const socket = this.replServer.createConnection(this.getUrl());
        socket.on('data', d => {
          this.iframe.runJs(`window.dispatchEvent(new MessageEvent('message', {data: '${d.toString('base64')}'}));`);
        });
        iframe.onconsole = m => {
          const match = m.match(/^POSTMESSAGE:([\s\S]+)$/);
          if (match) {
            const bMessage = Buffer.from(match[1], 'utf8');
            const b = Buffer.from(new ArrayBuffer(Uint32Array.BYTES_PER_ELEMENT + bMessage.byteLength));
            new Uint32Array(b.buffer, b.byteOffset, 1)[0] = bMessage.byteLength;
            b.set(bMessage, Uint32Array.BYTES_PER_ELEMENT);
            socket.write(b);
          }
        };
      };
      iframe.src = this.getUrl();
      return iframe;
    })();

    this.onRepl = this.onRepl.bind(this);
    this.replServer.on('repl', this.onRepl);
  }

  getUrl() {
     return `${this.replServer.url}&id=${this.id}`;
  }

  onRepl(r) {
    if (r.id === this.id) {
      r.setEval((s, context, filename, cb) => {
        let err = null, result;
        try {
          result = this.context.vm.run(s);
        } catch (e) {
          err = e;
        }
        if (!err) {
          cb(null, result);
        } else {
          cb(err);
        }
      });
    }
  }
  
  getIframe() {
    return this.iframe;
  }

  destroy() {
    this.replServer.removeListener('repl', this.onRepl);

    for (let i = 0; i < this.repls.length; i++) {
      this.repls[i].close();
    }
    this.repls.length = 0;
  }
}

module.exports = {
  async requestDevTools(context, document) {
    const replServer = await _getReplServer();
    return new DevTools(context, document, replServer);
  },
};

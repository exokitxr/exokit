const http = require('http');
const htermRepl = require('hterm-repl');

const DOM = require('./DOM');
const {HTMLIframeElement} = DOM;

const DEVTOOLS_PORT = 9223;

const _getReplServer = (() => {
  let replServer = null;
  return () => new Promise((accept, reject) => {
    if (!replServer) {
      htermRepl({
        port: DEVTOOLS_PORT,
      }, (err, newReplServer) => {
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

    this.onRepl = this.onRepl.bind(this);
    this.replServer.on('repl', this.onRepl);
  }

  /* getPath() {
    return `/?id=${this.id}`;
  } */
  getUrl() {
     return `${this.replServer.url}&id=${this.id}`;
    // return `http://127.0.0.1:${DEVTOOLS_PORT}${this.getPath()}`;
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

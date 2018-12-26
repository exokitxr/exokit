const http = require('http');
const htermRepl = require('hterm-repl');

const DOM = require('./DOM');
const {HTMLIframeElement} = DOM;

const DEVTOOLS_PORT = 9223;

let replServer = null;
const _getReplServer = () => {
  if (!replServer) {
    replServer = htermRepl({
      port: DEVTOOLS_PORT,
    });
  }
  return replServer;
};

let id = 0;
class DevTools {
  constructor(context) {
    this.context = context;
    this.id = (++id) + '';
    this.repls = [];

    const replServer = _getReplServer();
    this.onRepl = this.onRepl.bind(this);
    replServer.on('repl', this.onRepl);
  }

  getPath() {
    return `/?id=${this.id}`;
  }
  getUrl() {
    return `http://127.0.0.1:${DEVTOOLS_PORT}${this.getPath()}`;
  }

  onRepl(r) {
    if (r.url === this.getPath()) {
      r.setEval((s, context, filename, cb) => {
        let err = null, result;
        try {
          result = (function() {
            return eval(s);
          }).call(this.context);
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

  destroy() {
    const replServer = _getReplServer();
    replServer.removeListener('repl', this.onRepl);

    for (let i = 0; i < this.repls.length; i++) {
      this.repls[i].close();
    }
    this.repls.length = 0;
  }
}

module.exports = {
  createDevTools(iframe) {
    return new DevTools(iframe);
  },
};

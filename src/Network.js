throw new Error('fail');

/* const fs = require('fs');
const stream = require('stream');
const {XMLHttpRequest: XMLHttpRequestBase} = require('window-xhr');
const XHRUtils = require('window-xhr/lib/utils');

const {urls} = require('./urls');

const XMLHttpRequest = (Old => class XMLHttpRequest extends Old {
  open(method, url, async, username, password) {
    const blob = urls.get(url);
    if (blob) {
      this._properties._responseFn = cb => {
        process.nextTick(() => {
          const {buffer} = blob;
          const response = new stream.PassThrough();
          response.statusCode = 200;
          response.headers = {
            'content-length': buffer.length + '',
          };
          response.end(buffer);
          cb(response);
        });
      };
    } else {
      const match = url.match(/^file:\/\/(.*)$/);
      if (match) {
        const p = match[1];
        this._properties._responseFn = cb => {
          fs.lstat(p, (err, stats) => {
            if (!err) {
              const response = fs.createReadStream(p);
              response.statusCode = 200;
              response.headers = {
                'content-length': stats.size + '',
              };
              cb(response);
            } else if (err.code === 'ENOENT') {
              const response = new stream.PassThrough();
              response.statusCode = 404;
              response.headers = {};
              response.end('file not found: ' + p);
              cb(response);
            } else {
              const response = new stream.PassThrough();
              response.statusCode = 500;
              response.headers = {};
              response.end(err.stack);
              cb(response);
            }
          });
        };
        arguments[1] = 'http://127.0.0.1/'; // needed to pass protocol check, will not be fetched
      }
    }

    return Old.prototype.open.apply(this, arguments);
  }
})(XMLHttpRequestBase);
module.exports.XMLHttpRequest = XMLHttpRequest;

XHRUtils.createClient = (createClient => function() {
  const properties = arguments[0];
  if (properties._responseFn) {
    const cb = arguments[2];
    properties._responseFn(cb);
    return {
      on() {},
      setHeader() {}, write() {},
      end() {},
    };
  } else {
    return createClient.apply(this, arguments);
  }
})(XHRUtils.createClient); */
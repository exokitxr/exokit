const url = require('url');

const fetch = require('window-fetch');
const {_makeWindowVm} = require('./WindowVm');
const utils = require('./utils');
const GlobalContext = require('./GlobalContext');

GlobalContext.args = {};
GlobalContext.version = '';
GlobalContext.styleEpoch = 0;

let nativeWorker = null;
let nativeVr = GlobalContext.nativeVr = null;
let nativeMl = GlobalContext.nativeMl = null;
let nativeWindow = null;

const core = (htmlString = '', options = {}) => {
  options.url = options.url || 'http://127.0.0.1/';
  options.baseUrl = options.baseUrl || options.url;
  options.dataPath = options.dataPath || __dirname;
  return _makeWindowVm(htmlString, options);
};
core.load = (src, options = {}) => {
  if (!url.parse(src).protocol) {
    src = 'http://' + src;
  }
  let redirectCount = 0;
  const _fetchTextFollow = src => fetch(src, {
    redirect: 'manual',
  })
    .then(res => {
      if (res.status >= 200 && res.status < 300) {
        return res.text()
          .then(htmlString => ({
            src,
            htmlString,
          }));
      } else if (res.status >= 300 && res.status < 400) {
        const l = res.headers.get('Location');
        if (l) {
          if (redirectCount < 10) {
            redirectCount++;
            return _fetchTextFollow(l);
          } else {
            return Promise.reject(new Error('fetch got too many redirects: ' + res.status + ' : ' + src));
          }
        } else {
          return Promise.reject(new Error('fetch got redirect with no location header: ' + res.status + ' : ' + src));
        }
      } else {
        return Promise.reject(new Error('fetch got invalid status code: ' + res.status + ' : ' + src));
      }
    });
  return _fetchTextFollow(src)
    .then(({src, htmlString}) => {
      let baseUrl;
      if (options.baseUrl) {
        baseUrl = options.baseUrl;
      } else {
        baseUrl = utils._getBaseUrl(src);
      }

      return core(htmlString, {
        url: options.url || src,
        baseUrl,
        dataPath: options.dataPath,
      });
    });
};
core.setArgs = newArgs => {
  GlobalContext.args = newArgs;
};
core.setVersion = newVersion => {
  GlobalContext.version = newVersion;
};
module.exports = core;

/* if (require.main === module) {
  if (process.argv.length === 3) {
    const baseUrl = 'file://' + __dirname + '/';
    const u = new URL(process.argv[2], baseUrl).href;
    core.load(u);
  }
} */

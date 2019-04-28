const url = require('url');
const {URL} = url;

const fetch = require('window-fetch');
const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');
const {_getBaseUrl, _download} = require('./utils');
const {_makeWindow} = require('./WindowVm');

const exokit = module.exports;
exokit.make = (htmlString, options) => {
  if (typeof htmlString === 'object') {
    options = htmlString;
    htmlString = undefined;
  }
  htmlString = htmlString || '';
  options = options || {};

  options.url = options.url || 'http://127.0.0.1/';
  options.baseUrl = options.baseUrl || options.url;
  options.dataPath = options.dataPath || __dirname;
  options.args = options.args || {};
  options.htmlString = htmlString;
  options.replacements = options.replacements || {};
  return _makeWindow(options);
};
exokit.load = (src, options = {}) => {
  if (!url.parse(src).protocol) {
    src = 'http://' + src;
  }
  options.args = options.args || {};
  options.replacements = options.replacements || {};

  let redirectCount = 0;
  const _fetchTextFollow = src => fetch(src, {
    redirect: 'manual',
  })
    .then(res => {
      if (res.status >= 200 && res.status < 300) {
        return res.text()
          .then(t => {
            if (options.args.download) {
              return _download('GET', src, t, t => Buffer.from(t, 'utf8'), options.args.download);
            } else {
              return Promise.resolve(t);
            }
          })
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
        baseUrl = _getBaseUrl(src);
      }

      return exokit.make(htmlString, {
        url: options.url || src,
        baseUrl,
        dataPath: options.dataPath,
        args: options.args,
        replacements: options.replacements,
      });
    });
};
exokit.download = (src, dst) => exokit.load(src, {
  args: {
    download: dst,
    headless: true,
  },
})
  .then(window => new Promise((accept, reject) => {
    window.document.resources.addEventListener('drain', () => {
      accept();
    });
  }));
exokit.setArgs = newArgs => {
  GlobalContext.args = newArgs;
};
exokit.setVersion = newVersion => {
  GlobalContext.version = newVersion;
};

if (require.main === module) {
  if (process.argv.length === 3) {
    const baseUrl = 'file://' + __dirname + '/';
    const u = new URL(process.argv[2], baseUrl).href;
    exokit.load(u);
  }
}

const url = require('url');

const fetch = require('window-fetch');

const GlobalContext = require('./GlobalContext');

GlobalContext.args = {};
GlobalContext.version = '';

// Class imports.
const {_makeWindowVm} = require('./WindowVm');
const {Event, EventTarget} = require('./Event');
const utils = require('./utils');

GlobalContext.styleEpoch = 0;

const maxParallelResources = 8;
class Resource {
  constructor(getCb = (onprogress, cb) => cb(), value = 0.5, total = 1) {
    this.getCb = getCb;
    this.value = value;
    this.total = total;
    
    this.onupdate = null;
  }

  setProgress(value) {
    this.value = value;

    this.onupdate && this.onupdate();
  }
  
  get() {
    return new Promise((accept, reject) => {
      this.getCb(progress => {
        this.setValue(progress);
      }, err => {
        if (!err) {
          accept();
        } else {
          reject(err);
        }
      });
    });
  }
  
  destroy() {
    this.setProgress(1);
  }
}
class Resources extends EventTarget {
  constructor() {
    super();
    
    this.resources = [];
    this.queue = [];
    this.numRunning = 0;
  }

  getValue() {
    let value = 0;
    for (let i = 0; i < this.resources.length; i++) {
      value += this.resources[i].value;
    }
    return value;
  }
  getTotal() {
    let total = 0;
    for (let i = 0; i < this.resources.length; i++) {
      total += this.resources[i].total;
    }
    return total;
  }
  getProgress() {
    let value = 0;
    let total = 0;
    for (let i = 0; i < this.resources.length; i++) {
      const resource = this.resources[i];
      value += resource.value;
      total += resource.total;
    }
    return total > 0 ? (value / total) : 1;
  }

  addResource(getCb) {
    return new Promise((accept, reject) => {
      const resource = new Resource(getCb);
      resource.onupdate = () => {
        if (resource.value >= resource.total) {
          this.resources.splice(this.resources.indexOf(resource), 1);
          
          resource.onupdate = null;
          
          accept();
        }

        const e = new Event('update');
        e.value = this.getValue();
        e.total = this.getTotal();
        e.progress = this.getProgress();
        this.dispatchEvent(e);
      };
      this.resources.push(resource);
      this.queue.push(resource);
      
      this.drain();
    });
  }
  
  drain() {
    if (this.queue.length > 0 && this.numRunning < maxParallelResources) {
      const resource = this.queue.shift();
      resource.get()
        .catch(err => {
          console.warn(err.stack);
        })
        .finally(() => {
          resource.destroy();
          
          this.numRunning--;
          
          this.drain();
        });
      
      this.numRunning++;
    }
  }
}
GlobalContext.Resources = Resources;

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

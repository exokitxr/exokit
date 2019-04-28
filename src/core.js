const url = require('url');
const {URL} = url;

const {_makeWindowWithDocument} = require('./Window.js');

const fetch = require('window-fetch');

const THREE = require('../lib/three-min.js');

const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');

const {Event, EventTarget} = require('./Event');
const utils = require('./utils');
const {_download} = utils;

GlobalContext.args = {};
GlobalContext.version = '';

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
    } else {
      const _isDone = () => this.numRunning === 0 && this.queue.length === 0;
      if (_isDone()) {
        process.nextTick(() => { // wait one tick for more resources before emitting drain
          if (_isDone()) {
            this.emit('drain');
          }
        });
      }
    }
  }
}
GlobalContext.Resources = Resources;

const _fromAST = (node, window, parentNode, document, uppercase) => {
  if (node.nodeName === '#text') {
    const text = new window.Text(node.value);
    text.parentNode = parentNode;
    return text;
  } else if (node.nodeName === '#comment') {
    const comment = new window.Comment(node.data);
    comment.parentNode = parentNode;
    return comment;
  } else {
    let {tagName} = node;
    if (tagName && uppercase) {
      tagName = tagName.toUpperCase();
    }
    let {attrs, value, content, childNodes, sourceCodeLocation} = node;
    const HTMLElementTemplate = window[symbols.htmlTagsSymbol][tagName];
    const location = sourceCodeLocation  ? {
      line: sourceCodeLocation.startLine,
      col: sourceCodeLocation.startCol,
    } : null;
    const element = HTMLElementTemplate ?
      new HTMLElementTemplate(
        attrs,
        value,
        location,
      )
    :
      new window.HTMLElement(
        tagName,
        attrs,
        value,
        location,
      );
    element.parentNode = parentNode;
    if (!document) { // if there is no document, it's us
      document = element;
      document.defaultView = window;
      window.document = document;
    }
    if (content) {
      element.childNodes = new window.NodeList(
        content.childNodes.map(childNode =>
          _fromAST(childNode, window, element, document, uppercase)
        )
      );
    } else if (childNodes) {
      element.childNodes = new window.NodeList(
        childNodes.map(childNode =>
          _fromAST(childNode, window, element, document, uppercase)
        )
      );
    }
    return element;
  }
};
GlobalContext._fromAST = _fromAST;

function _upgradeElement(window, el, upgradeTagName) {
  const constructor = window.customElements.get(upgradeTagName);
  constructor && window.customElements.upgrade(el, constructor);
}

// To "run" the HTML means to walk it and execute behavior on the elements such as <script src="...">.
// Each candidate element exposes a method on runSymbol which returns whether to await the element load or not.
const _runHtml = (element, window) => {
  if (element instanceof window.HTMLElement) {
    return new Promise((accept, reject) => {
      const {document} = window;

      element.traverse(el => {
        const {id} = el;
        if (id) {
          el._emit('attribute', 'id', id, null);
        }

        if (el[symbols.runSymbol]) {
          document[symbols.addRunSymbol](el[symbols.runSymbol].bind(el));
        }

        const {tagName} = el;
        if (tagName) {
          if (/\-/.test(tagName)) {
            _upgradeElement(window, el, tagName);
          } else {
            const isAttr = el.getAttribute('is');
            if (isAttr) {
              _upgradeElement(window, el, isAttr);
            }
          }
        }
      });
      if (document[symbols.runningSymbol]) {
        document.once('flush', () => {
          accept();
        });
      } else {
        accept();
      }
    });
  } else {
    return Promise.resolve();
  }
};
GlobalContext._runHtml = _runHtml;

const exokit = module.exports;
exokit.make = (s = '', options = {}) => {
  options.url = options.url || 'http://127.0.0.1/';
  options.baseUrl = options.baseUrl || options.url;
  options.dataPath = options.dataPath || __dirname;
  options.args = options.args || {};
  options.replacements = options.replacements || {};
  return _makeWindowWithDocument(s, options);
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
        baseUrl = utils._getBaseUrl(src);
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
exokit.THREE = THREE;
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

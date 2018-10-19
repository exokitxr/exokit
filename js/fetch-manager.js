window.FetchManager = (() => {

const cache = new Map();
const _getCachedFile = u => {
  if (/^cache:/.test(u)) {
    const fileBlobUrl = cache.get(u);

    if (fileBlobUrl) {
      return {
        url: fileBlobUrl,
      };
    } else {
      return {
        error: {
          status: 404,
        },
      };
    }
  } else {
    return null;
  }
};
let id = 0;

window.fetch = (fetch => function(url, opts) {
  const cachedFile = _getCachedFile(url);
  if (cachedFile) {
    if (!cachedFile.error) {
      url = cachedFile.url;
      return fetch.apply(this, arguments);
    } else {
      const {status} = err;
      const res = new Response(null, {
        status,
      });
      return Promise.resolve(res);
    }
  } else {
    return fetch.apply(this, arguments);
  }
})(window.fetch);
window.XMLHttpRequest.prototype.open = (open => function(method, url, opts) {
  const cachedFile = _getCachedFile(url);
  if (cachedFile) {
    if (!cachedFile.error) {
      url = cachedFile.url;
      return open.apply(this, arguments);
    } else {
      const {status} = err;
      const res = new Response(null, {
        status,
      });
      return Promise.resolve(res);
    }
  } else {
    return open.apply(this, arguments);
  }
})(window.XMLHttpRequest.prototype.open);

(() => {
  const imgSrcDescriptor = (() => {
    for (let prototype = Image.prototype; prototype; prototype = Object.getPrototypeOf(prototype)) {
      const descriptor = Object.getOwnPropertyDescriptor(prototype, 'src');
      if (descriptor) {
        return descriptor;
      }
    }
    return null;
  })();
  Object.defineProperty(Image.prototype, 'src', {
    get: imgSrcDescriptor.get,
    set(u) {
      const cachedFile = _getCachedFile(u);
      if (cachedFile && !cachedFile.error) {
        u = cachedFile.url;
      }
      this.setAttribute('src', u);
    },
  });
})();

class FetchManager {
  constructor() {}

  addDirectory(files) {
    const directoryUrl = 'cache:' + (id++);
    for (let i = 0; i < files.length; i++) {
      const file = files[i];
      cache.set(directoryUrl + '/' + file.name, file.getBlobUrl());
    }
    return directoryUrl;
  }
}

return FetchManager;

})();

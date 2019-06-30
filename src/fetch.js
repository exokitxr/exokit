const url = require('url');
const windowFetch = require('window-fetch');

const protocols = {};
['http', 'https', 'data', 'blob'].forEach(p => {
  protocols[p] = windowFetch;
});

function fetch(u, options) {
  const protocol = url.parse(u).protocol.match(/^([^:]*):/)[1];
  if (protocol in protocols) {
    return protocols[protocol](u, options);
  } else {
    return Promise.reject(new Error('unknown protocol'));
  }
}
for (const k in windowFetch) {
  fetch[k] = windowFetch[k];
}

function registerProtocolHandler(protocol, protocolFetchImpl) {
  protocols[protocol] = protocolFetchImpl;
}

module.exports = {
  fetch,
  registerProtocolHandler,
};

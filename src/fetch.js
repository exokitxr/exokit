const {URL} = require('url');;
const utils = require('./utils');
const windowFetch = require('window-fetch');
const {Response} = windowFetch;

const protocols = {};
['http', 'https', 'data', 'blob'].forEach(p => {
  protocols[p] = windowFetch;
});

let baseUrl = undefined;

const PROTOCOL_REGEX = /^([^:]*):/

async function fetch(u, options) {
  let url = u
  if (typeof u === 'string') {
    const blob = URL.lookupObjectURL(u);
    if (blob) {
      return new Response(blob);
    } else {
      u = utils._normalizeUrl(u, baseUrl);
    }
  } else {
    // Fetch is being called with request
    // We should get the URL for matching the protoco
    url = u.url
  }

  const protocol = url.match(PROTOCOL_REGEX)[1];
  if (protocol in protocols) {
    return protocols[protocol](u, options);
  } else {
    throw new Error('unknown protocol');
  }
}

// Add expected fetch internals
for (const k in windowFetch) {
  fetch[k] = windowFetch[k];
}

function registerProtocolHandler(protocol, protocolFetchImpl) {
  protocols[protocol] = protocolFetchImpl;
}

function setBaseFetchUrl(url) {
  baseUrl = url;
}

module.exports = {
  fetch,
  registerProtocolHandler,
  setBaseFetchUrl
};

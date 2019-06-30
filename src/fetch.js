const windowFetch = require('window-fetch');
const protoFetch = require('proto-fetch')

const protocols = {
  file: windowFetch,
  http: windowFetch,
  https: windowFetch
}

const fetch = protoFetch(protocols)

const {Request, Response, Headers, Blob} = windowFetch

Object.assign(fetch, {Request, Response, Headers, Blob})

function registerProtocol(protocol, fetchImpl) {
  protocols[protocol] = fetchImpl
}

module.exports = {
  fetch,
  registerProtocol
}

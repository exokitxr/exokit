const windowFetch = require('window-fetch')
const url = require('url')

// This error message is what Firefox says when you fetch an unknown protocol
const INVALID_PROTOCOL = 'NetworkError when attempting to fetch resource.'

const protocols = {
  file: windowFetch,
  http: windowFetch,
  https: windowFetch,
  data: windowFetch,
  blob: windowFetch
}

const fetch = async function(uri, options) {
  const parsed = url.parse(uri)
  const protocol = parsed.protocol.split(':')[0]

  if(!protocols[protocol]) {
    return new Promise.reject(new TypeError(INVALID_PROTOCOL))
  }

  return protocols[protocol](uri, options)
}

const {Request, Response, Headers, Blob} = windowFetch

Object.assign(fetch, {Request, Response, Headers, Blob})

function registerProtocol(protocol, fetchImpl) {
  protocols[protocol] = fetchImpl
}

module.exports = {
  fetch,
  registerProtocol
}

const {Console} = require('console');
const stream = require('stream');
const {
  nativeConsole,
} = require('./native-bindings');

const consoleStream = new stream.Writable();
consoleStream._write = (chunk, encoding, callback) => {
  nativeConsole.Log(chunk);
  callback();
};
consoleStream._writev = (chunks, callback) => {
  for (let i = 0; i < chunks.length; i++) {
    nativeConsole.Log(chunks[i]);
  }
  callback();
};
global.console = new Console(consoleStream);

const {fetch} = require('./fetch');
const {workerData} = require('worker_threads');
const {url, int32Array} = workerData;

(async () => {
  const res = await fetch(url);
  if (res.status >= 200 && res.status < 300) {
    const arrayBuffer = await res.arrayBuffer();
    const srcBuffer = new Uint8Array(arrayBuffer, 0, arrayBuffer.byteLength);
    const dstBuffer = new Uint8Array(int32Array.buffer, int32Array.byteOffset + Int32Array.BYTES_PER_ELEMENT*3, int32Array.byteLength - Int32Array.BYTES_PER_ELEMENT*3);
    if (dstBuffer.byteLength >= srcBuffer.byteLength) {
      dstBuffer.set(srcBuffer);
      int32Array[1] = res.status;
      int32Array[2] = arrayBuffer.byteLength;
    } else {
      int32Array[1] = 0;
      int32Array[2] = 0;
    }
  } else {
    int32Array[1] = res.status;
    int32Array[2] = 0;
  }
})().catch(err => {
  console.warn('request error', err.stack);

  int32Array[1] = 0;
  int32Array[2] = 0;
}).finally(() => {
  int32Array[0] = 1;
  Atomics.notify(int32Array, 0);
});

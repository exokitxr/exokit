const path = require('path');
const child_process = require('child_process');

const electron = require('electron');

const cp = child_process.spawn(electron, [path.join(__dirname, 'electron-child.js')]);

const bs = [];
let bsLength = 0;
const _pull  = l => {
  if (bsLength >= l) {
    const result = Buffer.allocUnsafe(l);
    let localLength = 0;

    while (localLength < l) {
      const need = l - localLength;
      let b = bs.shift();
      if (b.length >= need) {
        b.copy(result, localLength, 0, need);
        b = b.slice(need);
        if (b.length > 0) {
          bs.unshift(b);
        }
        localLength += need;
        bsLength -= need;
      } else {
        b.copy(result, localLength, 0, b.length);
        localLength += b.length;
        bsLength -= b.length;
      }
    }

    return result;
  } else {
    return null;
  }
};
cp.stdout.on('data', b => {
  bs.push(b);
  bsLength += b.byteLength;

  while (bsLength >= 4*Uint32Array.BYTES_PER_ELEMENT) {
    let [b] = bs;
    if (b.length < 4) {
      console.warn(new Error('did not get full header from electron pipe').stack);
    }
    const header = new Uint32Array(b.buffer, b.byteOffset, 4);
    const [x, y, width, height] = header;
    // console.log('got header', x, y, width, height);
    b = _pull(4*Uint32Array.BYTES_PER_ELEMENT + width*height*4);
    if (b) {
      const imageData = Buffer.from(b.buffer, b.byteOffset + 4*Uint32Array.BYTES_PER_ELEMENT, width*height*4);
      console.log('got frame', x, y, width, height, imageData.byteLength);
    } else {
      break;
    }
  }
  // console.log('got data', b.length);
});
cp.stdout.on('end', () => {
  console.log('stdout end');
});
cp.stderr.on('data', b => {
  process.stdout.write(b);
});
cp.stderr.on('end', () => {
  console.log('stderr end');
});

process.on('SIGINT', () => {
  // cp.kill(9);
  console.log('SIGINT...');
  process.exit();
});
process.on('SIGTERM', () => {
  // cp.kill(9);
  console.log('SIGTERM...');
  process.exit();
});
process.on('exit', () => {
  console.log('exiting...');
  cp.kill(9);
});
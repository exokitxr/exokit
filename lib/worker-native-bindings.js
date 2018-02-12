const {nativeImageBitmap} = require(process.argv[2]);

module.exports = {
  createImageBitmap: image => Promise.resolve(nativeImageBitmap.createImageBitmap(image)),
};

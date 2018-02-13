const {nativeImageBitmap} = require(process.argv[2]);

module.exports = {
  ImageBitmap: nativeImageBitmap,
  createImageBitmap: image => Promise.resolve(nativeImageBitmap.createImageBitmap(image)),
};

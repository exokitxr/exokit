const {nativeImageBitmap} = require(process.argv[2]);

module.exports = {
  ImageBitmap: nativeImageBitmap,
  createImageBitmap: function() {
    return Promise.resolve(nativeImageBitmap.createImageBitmap.apply(nativeImageBitmap, arguments));
  }
};

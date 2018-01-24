const UglifyJsPlugin = require('uglifyjs-webpack-plugin');

module.exports = {
  entry: "./index.js",
  output: {
    filename: "bundle.js",
    libraryTarget: 'commonjs2',
  },
  plugins: [
    new UglifyJsPlugin(),
  ],
  target: 'node',
};

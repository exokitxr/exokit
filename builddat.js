const browserify = require('browserify')
const defaultBuiltins = require('browserify/lib/builtins')


browserify('./node_modules/node-dat-archive/', {
  // builtins: {
  //   process: require.resolve('process/browser')
  // },
  insertGlobalVars: {
    process: function() {return 'require("process/browser.js")' }
  },
  standalone: 'DatArchive',
  // builtins: false,
  browserField: false,
  detectGlobals: true,
})
.exclude(Object.keys(defaultBuiltins))
.exclude('utp-native')
.bundle()
.pipe(process.stdout)

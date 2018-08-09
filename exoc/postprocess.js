const path = require('path');
const fs = require('fs');

(() => {
  const srcPath = path.join(__dirname, 'out', 'main.c');
  let src = fs.readFileSync(srcPath, 'utf8');
  src = src.replace(/static u32 main\(/gm, 'u32 main(');
  fs.writeFileSync(srcPath, src);
})();
(() => {
  const srcPath = path.join(__dirname, 'out', 'wasm-rt.h');
  let src = fs.readFileSync(srcPath, 'utf8');
  src = src.replace(/(#define WASM_RT_H_\n)/gm, `$1
    #if _WIN32
      #define __attribute__(A) /* nothing */
      #define __builtin_expect(A,B) (A)
    #endif
  `);
  fs.writeFileSync(srcPath, src);
})();

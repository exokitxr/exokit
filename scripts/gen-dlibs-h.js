#!/usr/bin/env node

const path = require('path');
const find = require('find');

const dirname = path.resolve(__dirname, '..');
const directories = ['build', 'node_modules'];

find.file(/\.node$/, dirname, files => {
  files = files.filter(file => directories.some(directory => file.indexOf(path.join(dirname, directory)) === 0));
  const header = `#include <v8.h>\n#include <node_api.h>\n\n`;
  let decls = `extern "C" {\n`;
  let registers = `inline void registerDlibs(std::map<std::string, std::pair<void *, bool>> &dlibs) {\n`;

  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    if (!/obj\.target/.test(file)) {
      const match = file.match(/^(.+\/)([^\/]+)\.node$/);
      if (match) {
        const relpath = match[1].slice(dirname.length);
        const binName = match[2];
        const npmName = (() => {
          const match = relpath.match(/\/node_modules\/([^\/]+)\/build\/(?:Release|Debug)\/$/);
          return match && match[1];
        })();
        // ignore incompatible modules
        if (!npmName || (npmName.replace(/-/g, '_') === binName || (npmName.replace(/-/g, '_') + '2') === binName)) {
          const registerName = `node_register_module_${binName}`;
          decls += `  void ${registerName}(Local<Object> exports, Local<Value> module, Local<Context> context);\n`;
          registers += `  dlibs["/package${relpath}${binName}.node"] = std::pair<void *, bool>((void *)&${registerName}, false);\n`;
        } else if (npmName && npmName.replace(/-/g, '_') + '_napi' === binName) {
          const registerName = `node_register_module_${binName}`;
          decls += `  napi_value ${registerName}(napi_env env, napi_value exports);\n`;
          registers += `  dlibs["/package${relpath}${binName}.node"] = std::pair<void *, bool>((void *)&${registerName}, true);\n`;
        }
      }
    }
  }
  
  decls += `}\n`;
  registers += `}\n`;
  
  console.log(header);
  console.log(decls);
  console.log(registers);
});

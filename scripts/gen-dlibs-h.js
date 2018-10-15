#!/usr/bin/env node

const find = require('find');

const dirname = process.argv[2];

find.file(/\.node$/, dirname, files => {
  let decls = `extern "C" {\n`;
  let registers = `inline void registerDlibs(std::map<std::string, void *> &dlibs) {\n`;

  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    if (!/obj\.target/.test(file)) {
      const match = file.match(/^(.+\/)([^\/]+)\.node$/);
      if (!match) { continue; }
      const relpath = match[1].slice(dirname.length);
      const binName = match[2];
      const npmName = (() => {
        const match = relpath.match(/\/node_modules\/([^\/]+)/);
        return match && match[1];
      })();
      if (!npmName || npmName.replace(/-/g, '_') === binName) { // ignore incompatible modules
        const registerName = `node_register_module_${binName}`;
        decls += `  void ${registerName}(Local<Object> exports, Local<Value> module, Local<Context> context);\n`;
        registers += `  dlibs["/package${relpath}${binName}.node"] = (void *)&${registerName};\n`;
      }
    }
  }
  
  decls += `}\n`;
  registers += `}\n`;
  
  console.log(decls);
  console.log(registers);
});

#!/usr/bin/env node

const find = require('find');

find.file(/\.node$/, __dirname, files => {
  let decls = `extern "C" {\n`;
  let registers = `inline void registerDlibs(std::map<std::string, void *> &dlibs) {\n`;

  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    if (!/obj\.target/.test(file)) {
      const match = file.match(/^(.+\/)([^\/]+)\.node$/);
      const relpath = match[1].slice(__dirname.length);
      const name = match[2];
      const registerName = `node_register_module_${name}`;
      decls += `  void ${registerName}(Local<Object> exports, Local<Value> module, Local<Context> context);\n`;
      registers += `  dlibs["/package${relpath}${name}.node"] = (void *)&${registerName};\n`;
    }
  }
  
  decls += `}\n`;
  registers += `}\n`;
  
  console.log(decls);
  console.log(registers);
});

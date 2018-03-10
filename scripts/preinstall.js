const path = require('path');
const fs = require('fs');
const child_process = require('child_process');

const rootPath = path.join(__dirname, '..');

const submodulePaths = [
  path.join(rootPath, 'deps', 'exokit-bindings'),
  path.join(rootPath, 'deps', 'glfw'),
];

if (submodulePaths.some(submodulePath => !fs.existsSync(submodulePath))) {
  child_process.execFileSync('git', ['submodule', 'init'], {
    cwd: rootPath,
  });
  child_process.execFileSync('git', ['submodule', 'update'], {
    cwd: rootPath,
  });
}

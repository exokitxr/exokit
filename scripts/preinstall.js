const path = require('path');
const fs = require('fs');
const child_process = require('child_process');
const rimraf = require('rimraf');

// if submodules were not checked out for some reason, manually check them out

const rootPath = path.join(__dirname, '..');
const submodulePaths = [
  path.join(rootPath, 'deps', 'exokit-bindings', '.git'),
  path.join(rootPath, 'deps', 'glfw', '.git'),
];

if (submodulePaths.some(submodulePath => !fs.existsSync(submodulePath))) {
  const exokitBindingsPath = path.join(rootPath, 'deps', 'exokit-bindings');
  rimraf.sync(exokitBindingsPath);
  child_process.execFileSync('git', ['clone', '--depth=1', 'https://github.com/modulesio/exokit-bindings', exokitBindingsPath], {
    cwd: rootPath,
    stdio: 'inherit',
  });

  const glfwPath = path.join(rootPath, 'deps', 'glfw');
  rimraf.sync(glfwPath);
  child_process.execFileSync('git', ['clone', '--depth=1', 'https://github.com/glfw/glfw', glfwPath], {
    cwd: rootPath,
    stdio: 'inherit',
  });
}

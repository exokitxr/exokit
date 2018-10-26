const path = require('path');
const fs = require('fs');
const childProcess = require('child_process');

const packageJson = JSON.parse(fs.readFileSync(path.join(__dirname, '../', 'package.json')));
const dependencies = Object.keys(packageJson.dependencies);
const nativeDependencies = [...dependencies.filter(dependency => /^native-/.test(dependency))]

for (let i = 0; i < nativeDependencies.length; i++) {
  childProcess.execSync('npm run postinstall .', {
    cwd: path.dirname(require.resolve(nativeDependencies[i])),
  });
}

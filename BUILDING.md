# Building Exokit

Exokit is a node module. The core is written in Javascript, with native bindings to native functionality like OpenGL. It downloads prebuilt binaries for libraries, which are listed as dependenices.

## Windows

**Requirements**: Node.js 10, Visual Studio 2015 or 2017

### Procedure
1. open **Node.js command prompt**
1. `git clone https://github.com/webmixedreality/exokit.git`
1. run `npm install`

## MacOS

**Requirements**: Node.js 10, XCode 

### Procedure
1. open **Terminal.app**
1. `git clone https://github.com/webmixedreality/exokit.git`
1. run `npm install`

## Linux

**Requirements**: Node.js 10

### Procedure
1. open **bash**
1. install dependencies: `apt-get install -y build-essential wget python libglfw3-dev libglew-dev libfreetype6-dev libfontconfig1-dev uuid-dev libxcursor-dev libxinerama-dev libxi-dev libasound2-dev libexpat1-dev`
1. `git clone https://github.com/webmixedreality/exokit.git`
1. run `npm install`

## Magic Leap

**Requirements**: Windows Subsystem for Linux, Ubuntu Bash, Node.js 10 for Windows, Visual studio 2017, Python 2, Python 3

### Procedure
1. get developer certificate from creator.magicleap.com
1. place developer certificate in `cert/{app.cert,app.privkey}`
1. open **Ubuntu Bash on Windows**
1. run `build-ml.sh`

# Building Exokit

Exokit is a node module. The core is written in Javascript, with native bindings to native functionality like OpenGL. It downloads prebuilt binaries for libraries, which are listed as dependenices.

## Windows

**Requirements**: Node.js 10, Python 2, Visual Studio 2015 or 2017

**Alternative requirements**: Node.js 10, as administrator: `npm install -g windows-build-tools`

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

**Requirements**: Windows Subsystem for Linux, Ubuntu Bash, Node.js 10.14.2 for Windows, Visual studio 2017, Python 2, Python 3

*NOTE: Due to combined usage of Ubuntu and Windows tools, you must check out ExoKit to somewhere reachable from both; the easiest way is to do your `git clone` using Windows, since for example Ubuntu can reach `C:\ExoKit` as `/mnt/c/ExoKit`.*

### Procedure (signed)
1. download Magic Leap Package Manager from https://creator.magicleap.com
1. install Magic Leap Lumin SDK
1. download developer certificate and private key from creator.magicleap.com
1. place certificate/private key in `cert/{app.cert,app.privkey}`
1. open **Ubuntu Bash on Windows**
1. `export MLSDK=/mnt/c/your_mlsdk_path_goes_here # fill this in`
1. run `scripts/build-ml.sh`

### Procedure (unsigned)
1. download Magic Leap Package Manager from https://creator.magicleap.com
1. install Magic Leap Lumin SDK
1. open **Ubuntu Bash on Windows**
1. `export MLSDK=/mnt/c/your_mlsdk_path_goes_here # fill this in`
1. `scripts/build-ml.sh --unsigned`

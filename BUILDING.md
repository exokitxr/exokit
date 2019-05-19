# Building Exokit

Exokit is a node module. The core is written in Javascript, with native bindings to native functionality like OpenGL. It downloads prebuilt binaries for libraries, which are listed as dependenices.

> Note: Using [nvm](https://github.com/creationix/nvm) makes it easy to manage multiple active node.js versions.

Build targets :

## Windows

**Requirements**: [Node.js 11.6.0](https://nodejs.org/dist/v11.6.0/), Python 2, Visual Studio 2015 or 2017

**Alternative requirements**: Node.js 11.6.0, as administrator: `npm install -g windows-build-tools`

### Procedure
1. open **Node.js command prompt**
1. `git clone https://github.com/exokitxr/exokit.git`
1. run `npm install`

## MacOS

**Requirements**: Node.js 11.6.0, XCode

### Procedure
1. open **Terminal.app**
1. `git clone https://github.com/exokitxr/exokit.git`
1. run `npm install`

## Linux

**Requirements**: Node.js 11.6.0

### Procedure
1. open **bash**
1. install dependencies: `apt-get install -y build-essential wget python libglfw3-dev libglew-dev libfreetype6-dev libfontconfig1-dev uuid-dev libxcursor-dev libxinerama-dev libxi-dev libasound2-dev libexpat1-dev`
1. `git clone https://github.com/exokitxr/exokit.git`
1. run `npm install`

## Magic Leap

**Requirements**: Windows Subsystem for Linux, Ubuntu Bash, Node.js 11.6.0 for Windows, Python 2, Python 3

> *NOTE: Due to combined usage of Ubuntu and Windows tools, you must check out Exokit to somewhere reachable from both; the easiest way is to do your `git clone` inside Ubuntu bash, since for example Ubuntu can reach `C:\exokit` as `/mnt/c/exokit`. (included in instructions below)*

### Procedure (signed)
1. download developer certificate and private key from https://creator.magicleap.com in Publish then Certificates
1. place the renamed certificate in `cert/app.cert` and private key in `cert/app.privkey`
1. download Magic Leap Package Manager from https://creator.magicleap.com
1. install Magic Leap Lumin SDK
1. open **Ubuntu Bash on Windows**
1. `cd /mnt/c/exokit`
1. `git clone https://github.com/exokitxr/exokit.git`
1. install dependencies: `sudo apt-get install -y build-essential wget python libglfw3-dev libglew-dev libfreetype6-dev libfontconfig1-dev uuid-dev libxcursor-dev libxinerama-dev libxi-dev libasound2-dev libexpat1-dev`
1. `export MLSDK=/mnt/c/your_mlsdk_path_goes_here # fill this in` e.g. `export MLSDK=/mnt/c/Users/Name/MagicLeap/mlsdk/v0.20.0/`
1. run `scripts/magicleap/build-ml.sh`

Note that, unlike for other target platform you should **not** run `npm install`. If you do remove the `node_modules/` directory and clean npm cache.

### Procedure (unsigned)
1. download Magic Leap Package Manager from https://creator.magicleap.com
1. install Magic Leap Lumin SDK
1. open **Ubuntu Bash on Windows**
1. `cd /mnt/c/exokit`
1. `git clone https://github.com/exokitxr/exokit.git`
1. install dependencies: `sudo apt-get install -y build-essential wget python libglfw3-dev libglew-dev libfreetype6-dev libfontconfig1-dev uuid-dev libxcursor-dev libxinerama-dev libxi-dev libasound2-dev libexpat1-dev`
1. `export MLSDK=/mnt/c/your_mlsdk_path_goes_here # fill this in` e.g. `export MLSDK=/mnt/c/Users/Name/MagicLeap/mlsdk/v0.20.0/`
1. run `scripts/magicleap/build-ml.sh --unsigned`

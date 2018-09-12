#!/bin/bash

# initialization

set -e

# arguments

ARG1=${1:---all}

# preface

cd "$(dirname "$0")"

export MLSDK=${MLSDK:-/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.16.0}
export MLSDK_WIN=$(echo "$MLSDK" | sed 's/^\/mnt\/c\//C:\\/' | sed 's/\//\\/g')

export CC="$MLSDK/tools/toolchains/bin/aarch64-linux-android-clang"
export CXX="$MLSDK/tools/toolchains/bin/aarch64-linux-android-clang++"
export LINK="$MLSDK/tools/toolchains/bin/aarch64-linux-android-clang++"
export AR="$MLSDK/tools/toolchains/bin/aarch64-linux-android-ar"

# pass down to child builds
EXTRA_FLAGS="-I$MLSDK/lumin/stl/libc++/include -I$MLSDK/lumin/usr/include"
export CFLAGS="$CFLAGS $EXTRA_FLAGS"
export CXXFLAGS="$CXXFLAGS $EXTRA_FLAGS"

export LUMIN=1

export npm_config_arch=arm64

if [ ! -d magicleap-js ]; then
  git clone https://github.com/webmixedreality/magicleap-js
else
  pushd magicleap-js
  git pull --rebase
  popd
fi

./magicleap-js/hack-toolchain.js

pushd ..
npm i --verbose --devdir="$(pwd)/.node-gyp" --arch=arm64 --target_arch=arm64
find -name '\.bin' | xargs rm -Rf
popd

# npm install libification

pushd ..
rm -Rf build/libexokit
mkdir -p build/libexokit
find build/Release/obj.target node_modules -name '*.o' | xargs "$AR" crs build/libexokit/libexokit.a
./scripts/gen-dlibs-h.js "$(pwd)" >build/libexokit/dlibs.h
popd

# build mpk

./magicleap-js/hack-toolchain.js -u

cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin ../metadata/program-device.mabu
if [ "$ARG1" = "--signed" ] || [ "$ARG1" = "--all" ]; then
  cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin -m ../metadata/manifest-device.xml -p --create-package -s ../cert/app.cert ../metadata/app-device.package
  cp ../build/magicleap/app-device/app-device.mpk ../build/magicleap/exokit.mpk
fi
if [ "$ARG1" = "--unsigned" ] || [ "$ARG1" = "--all" ]; then
  cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin -m ../metadata/manifest-device.xml -p --create-package --allow-unsigned ../metadata/app-device.package
  cp ../build/magicleap/app-device/app-device.mpk ../build/magicleap/exokit-unsigned.mpk
fi

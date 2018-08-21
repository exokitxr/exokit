#!/bin/bash

# cd /mnt/c/Users/avaer/Documents/GitHub/exokit

# preface

export MLSDK='/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.16.0'
export MLSDK_WIN='C:\Users\avaer\MagicLeap\mlsdk\v0.16.0'

export CC="$MLSDK/tools/toolchains/bin/aarch64-linux-android-clang"
export CXX="$MLSDK/tools/toolchains/bin/aarch64-linux-android-clang++"
export LINK="$MLSDK/tools/toolchains/bin/aarch64-linux-android-clang++"
export AR="$MLSDK/tools/toolchains/bin/aarch64-linux-android-ar"

# pass down to child builds
EXTRA_FLAGS="-I/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.16.0/lumin/stl/libc++/include -I/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.16.0/lumin/usr/include"
export CFLAGS="$CFLAGS $EXTRA_FLAGS"
export CXXFLAGS="$CXXFLAGS $EXTRA_FLAGS"

export LUMIN=1

export npm_config_arch=arm64

# npm install

../magicleap-js/hack-toolchain.js

rm -Rf node_modules
npm cache clean --force
npm i --verbose --devdir="$(pwd)/.node-gyp" --arch=arm64 --target_arch=arm64
find -name '\.bin' | xargs rm -Rf

# npm install libification

rm -Rf build/libexokit
mkdir -p build/libexokit
find build/Release/obj.target node_modules -name '*.o' | xargs "$AR" crs build/libexokit/libexokit.a
./gen-dlibs-h.js >build/libexokit/dlibs.h

# build mpk

../magicleap-js/hack-toolchain.js -u

rm -Rf build/magicleap
cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin program-device.mabu
cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin -m manifest-device.xml -s cert/app.privkey -p --create-package app-device.package
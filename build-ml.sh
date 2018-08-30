#!/bin/bash

# cd /mnt/c/Users/avaer/Documents/GitHub/exokit

# preface

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

../magicleap-js/hack-toolchain.js

npm i --verbose --devdir="$(pwd)/.node-gyp" --arch=arm64 --target_arch=arm64
find -name '\.bin' | xargs rm -Rf

# npm install libification

rm -Rf build/libexokit
mkdir -p build/libexokit
find build/Release/obj.target node_modules -name '*.o' | xargs "$AR" crs build/libexokit/libexokit.a
./gen-dlibs-h.js >build/libexokit/dlibs.h

# build mpk

../magicleap-js/hack-toolchain.js -u

cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin program-device.mabu
cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin -m manifest-device.xml -p --create-package -s cert/app.cert app-device.package
cp build/magicleap/app-device/app-device.mpk build/magicleap/exokit.mpk
cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin -m manifest-device.xml -p --create-package --allow-unsigned app-device.package
cp build/magicleap/app-device/app-device.mpk build/magicleap/exokit-unsigned.mpk

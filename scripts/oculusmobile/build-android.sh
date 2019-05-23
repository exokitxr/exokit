#!/bin/bash

NODE_VERSION=`node -v`
if [ $NODE_VERSION != "v11.15.0" ]
then
  echo "[ERROR] Node version needs to be v11.15.0."
  exit 1
fi

# Initialization.

set -e

# Preface.

TOOLCHAIN="$ANDROID_HOME/ndk-bundle/toolchain"
cd "$(dirname "$0")"

export TOOLCHAIN_USR="$TOOLCHAIN/sysroot/usr"
export TOOLCHAIN_LIB=$TOOLCHAIN/sysroot/usr/lib
export TOOLCHAIN_LIB_CXX=$TOOLCHAIN/aarch64-linux-android/lib
export TOOLCHAIN_LIB_64=$TOOLCHAIN/aarch64-linux-android/lib64
#export TOOLCHAIN_INCLUDE_LIB=$TOOLCHAIN/include/c++/4.9.x
export TOOLCHAIN_INCLUDE_SYSROOT=$TOOLCHAIN/sysroot/usr/include
export LD_LIBRARY_PATH="./Release:.:$LD_LIBRARY_PATH"

export CC_target="aarch64"
export GYP_CROSSCOMPILE=1
export CLANG_CXX_LIBRARY="libstdc++"
export CC="$TOOLCHAIN/bin/aarch64-linux-android-clang"
export CXX="$TOOLCHAIN/bin/aarch64-linux-android-clang++"
export LINK="$TOOLCHAIN/bin/aarch64-linux-android-clang++"
export AR="$TOOLCHAIN/bin/aarch64-linux-android-ar"
export LD="$TOOLCHAIN/bin/aarch64-linux-android-ld"

export OS="android"
export ANDROID=1

EXTRA_FLAGS="-I$TOOLCHAIN_INCLUDE_LIB -I$TOOLCHAIN_INCLUDE_SYSROOT"
export CFLAGS="$CFLAGS $EXTRA_FLAGS"
export CXXFLAGS="$CXXFLAGS $EXTRA_FLAGS"
export LDFLAGS=""

export npm_config_arch=aarch64
export OS="android"
export TARGET_ARCH="aarch64"  # For webrtc install prebuilt.

pushd ../../

# build native modules
npm install --verbose --devdir="$(pwd)/.node-gyp" --arch=aarch64 --target_arch=aarch64 --no-optional

# archive native module dlibs
mkdir -p build/libexokit
find build/Release/obj.target node_modules -name '*.o' | xargs "$AR" crs build/libexokit/libexokit.a
../gen-dlibs-h.js "$(pwd)" >build/libexokit/dlibs.h

# copy assets for packaging
mkdir -p ./android/app/assets
rsync -a --copy-links package.json ./android/app/assets/package.json
rsync -a --copy-links src/ ./android/app/assets/src
rsync -a --copy-links lib/ ./android/app/assets/lib
rsync -a --copy-links examples/ ./android/app/assets/examples
rsync -a --copy-links node_modules/* ./android/app/assets/node_modules/
rsync -a --relative --copy-links build/Release/exokit.node ./android/app/assets/
./scripts/gen-assets-h.js >android/app/assets/index.bin

# build apk
pushd android
./gradlew build && ./gradlew assembleDebug
popd

# for debugging symbols lookup
mkdir -p android/app/src/main/arm64-v8a
rsync -a --copy-links android/app/build/intermediates/cmake/debug/obj/arm64-v8a/libnative-main.so android/app/src/main/arm64-v8a/libnative-main.so

popd

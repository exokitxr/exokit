#!/bin/bash

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")"

TOOLCHAIN=~/Code/arm
export TOOLCHAIN_LIB=$TOOLCHAIN/sysroot/usr/lib
export TOOLCHAIN_LIB_CXX=$TOOLCHAIN/aarch64-linux-android/lib
export TOOLCHAIN_LIB_64=$TOOLCHAIN/aarch64-linux-android/lib64
export TOOLCHAIN_INCLUDE_LIB=$TOOLCHAIN/include/c++/4.9.x
export TOOLCHAIN_INCLUDE_SYSROOT=$TOOLCHAIN/sysroot/usr/include
export TOOLCHAIN_INCLUDE_EGL=$TOOLCHAIN/sysroot/usr/EGL

export CC="$TOOLCHAIN/bin/aarch64-linux-android-clang"
export CXX="$TOOLCHAIN/bin/aarch64-linux-android-clang++"
export CLANG_CXX_LIBRARY="libstdc++"
export LINK="$TOOLCHAIN/bin/aarch64-linux-android-clang++"
export PATH=$TOOLCHAIN/bin:$PATH

export ANDROID=1

EXTRA_FLAGS="-I$TOOLCHAIN_INCLUDE_LIB -I$TOOLCHAIN_INCLUDE_SYSROOT -L$TOOLCHAIN_LIB -L$TOOLCHAIN_LIB_CXX -L$TOOLCHAIN_LIB_64 -L$TOOLCHAIN_INCLUDE_EGL"
export CFLAGS="$CFLAGS $EXTRA_FLAGS"
export CXXFLAGS="$CXXFLAGS $EXTRA_FLAGS"

export npm_config_arch=arm64

pushd ..
export TARGET_ARCH="arm64"  # for webrtc install prebuilt

if test -z $NO_INSTALL
then
ANDROID=1 npm install --no-optional --verbose --ignore-scripts
node scripts/preinstall.js
fi

node-gyp rebuild --arch=arm64 --target_arch=arm64 --devdir="$(pwd)/.node-gyp" -v
find -name '\.bin' | xargs rm -Rf
popd

# pushd ..
# rm -Rf build/libexokit
# mkdir -p build/libexokit
# find build/Release/obj.target node_modules -name '*.o' | xargs "$AR" crs build/libexokit/libexokit.a
# ./scripts/gen-dlibs-h.js "$(pwd)" >build/libexokit/dlibs.h
# popd

# TODO: Build for Android, package with Gradle?

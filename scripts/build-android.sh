#!/bin/bash

# Initialization.

set -e

# Preface.

TOOLCHAIN="$PWD/node_modules/android-toolchain"
cd "$(dirname "$0")"

export TOOLCHAIN_USR="$TOOLCHAIN/sysroot/usr"
export TOOLCHAIN_LIB=$TOOLCHAIN/sysroot/usr/lib
export TOOLCHAIN_LIB_CXX=$TOOLCHAIN/aarch64-linux-android/lib
export TOOLCHAIN_LIB_64=$TOOLCHAIN/aarch64-linux-android/lib64
export TOOLCHAIN_INCLUDE_LIB=$TOOLCHAIN/include/c++/4.9.x
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

pushd ..
export TARGET_ARCH="aarch64"  # For webrtc install prebuilt.

if test -z $NO_INSTALL
then
ANDROID=1 npm install --no-optional --verbose --ignore-scripts --target_arch=aarch64
node scripts/preinstall.js
fi

OS=android node-gyp rebuild --arch=aarch64 --devdir="$(pwd)/.node-gyp" -v --loglevel=error --python="$TOOLCHAIN/bin/python2"

# Create archive.
rm -Rf build/libexokit
mkdir -p build/libexokit
find build/Release/obj.target node_modules -name '*.o' | xargs "$AR" crs build/libexokit/libexokit.a
./scripts/gen-dlibs-h.js "$(pwd)" >build/libexokit/dlibs.h
popd

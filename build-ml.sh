#!/bin/bash

cd /mnt/c/Users/avaer/Documents/GitHub/exokit

rm -Rf node_modules
npm cache clean --force

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

# set AR=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-ar.exe
# set RANLIB=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-ranlib.exe
# set LINK=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-ld.exe
# set CPP=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-g++.exe
# set STRIP=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-strip.exe
# set OBJCOPY=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-objcopy.exe
# set LD=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-ld.exe
# set OBJDUMP=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-objdump.exe
# set NM=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-nm.exe
# set AS=C:\Users\avaer\MagicLeap\mlsdk\v0.16.0\bin\aarch64-linux-android-as.exe
export npm_config_arch=arm64

npm i --verbose --devdir="$(pwd)/.node-gyp" --arch=arm64 --target_arch=arm64 --LUMIN=true

# unhack

cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin program-device.mabu
cmd.exe /c "$MLSDK_WIN/mabu.cmd" "MLSDK=$MLSDK_WIN" -v -t release_lumin -m manifest-device.xml -s cert/app.privkey -p --create-package --allow-unsigned app-device.package
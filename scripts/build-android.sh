#!/bin/bash

# Initialization.

set -e

# Preface.

export AR="/home/a/ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-ar"

cd "$(dirname "$0")/.."

rsync -a --copy-links package.json ./android/app/assets/package.json
rsync -a --copy-links src/ ./android/app/assets/src
rsync -a --copy-links lib/ ./android/app/assets/lib
rsync -a --copy-links examples/ ./android/app/assets/examples
rsync -a --copy-links node_modules/* ./android/app/assets/node_modules/
rsync -a --relative --copy-links build/Release/exokit.node ./android/app/assets/
./scripts/gen-assets-h.js >android/app/assets/index.bin 2>android/app/assets/index.txt

rm -Rf build/libexokit
mkdir -p build/libexokit
find build/Release/obj.target node_modules -name '*.o' | xargs "$AR" crs build/libexokit/libexokit.a
./scripts/gen-dlibs-h.js >build/libexokit/dlibs.h

pushd android
./gradlew build && ./gradlew assembleDebug
popd

#!/bin/bash

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")/.."

# pushd scripts
# ./gen-assets-h.js $(pwd)/.. >../build/libexokit/assets.h 2>../build/libexokit/assets.h2
# popd

rsync -a --copy-links package.json ./android/app/assets/package.json
rsync -a --copy-links src/ ./android/app/assets/src
rsync -a --copy-links lib/ ./android/app/assets/lib
rsync -a --copy-links examples/ ./android/app/assets/examples
rsync -a --copy-links node_modules/ ./android/app/assets/node_modules

# mkdir -p build/libexokit
pushd scripts
./gen-assets-h.js $(pwd)/.. >../android/app/assets/index.bin
popd

pushd android
./gradlew build && ./gradlew assembleDebug
popd

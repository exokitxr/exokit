#!/bin/bash

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")/.."

pushd scripts
./gen-assets-h.js $(pwd)/.. >../build/libexokit/assets.h
popd

rsync -Pa package.json ./android/app/assets/package.json
rsync -Pa src/ ./android/app/assets/src
rsync -Pa examples/ ./android/app/assets/examples

pushd android
./gradlew build && ./gradlew assembleDebug
popd

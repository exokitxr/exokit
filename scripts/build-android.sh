#!/bin/bash

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")/.."

pushd scripts
./gen-assets-h.js $(pwd)/.. >../build/libexokit/assets.h
popd

pushd android
./gradlew build && ./gradlew assembleDebug
popd

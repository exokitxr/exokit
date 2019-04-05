#!/bin/bash

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")/.."

pushd android
./gradlew build && ./gradlew assembleDebug
popd

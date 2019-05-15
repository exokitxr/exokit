#!/bin/bash

# Initialization.

set -e

# Preface.

source ./version-android.sh

cd "$(dirname "$0")/.."

export ANDROID_HOME=${ANDROID_HOME:-/home/a/sdk}

pushd android
"$ANDROID_HOME/platform-tools/adb" install -r ./app/build/outputs/apk/debug/app-debug.apk
# ./gradlew installDebug
popd

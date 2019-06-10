#!/bin/bash

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")"

source ./version-android.sh

pushd ../../android
"$ANDROID_HOME/platform-tools/adb" install -r ./app/build/outputs/apk/debug/app-debug.apk
# ./gradlew installDebug
popd

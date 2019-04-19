#!/bin/bash

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")/.."

pushd ./android/app/src/main
"$ANDROID_HOME/ndk-bundle/ndk-gdb" --adb="$ANDROID_HOME/platform-tools/adb" --launch=android.app.NativeActivity
popd

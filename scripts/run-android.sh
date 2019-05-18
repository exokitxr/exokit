#!/bin/bash

# Initialization.

set -e

# Preface.

source ./version-android.sh

cd "$(dirname "$0")/.."

pushd android

"$ANDROID_HOME/platform-tools/adb" shell am force-stop com.webmr.exokit
"$ANDROID_HOME/platform-tools/adb" logcat -c
"$ANDROID_HOME/platform-tools/adb" shell am start -n com.webmr.exokit/android.app.NativeActivity
# "$ANDROID_HOME/platform-tools/adb" shell am start -n com.webmr.exokit/android.app.NativeActivity -e ARGS "'node --experimental-worker /package /package/examples/tutorial.html'"
"$ANDROID_HOME/platform-tools/adb" logcat
popd

#!/bin/bash

# Initialization.

set -e

# Preface.


cd "$(dirname "$0")"

source ./version-android.sh

pushd ../../android
"$ANDROID_HOME/platform-tools/adb" shell am force-stop com.webmr.exokit
"$ANDROID_HOME/platform-tools/adb" logcat -c
"$ANDROID_HOME/platform-tools/adb" shell am start -n com.webmr.exokit/android.app.NativeActivity
# "$ANDROID_HOME/platform-tools/adb" shell am start -n com.webmr.exokit/android.app.NativeActivity -e ARGS "'node --inspect-brk /package /package/examples/tutorial.html'"
"$ANDROID_HOME/platform-tools/adb" logcat
popd

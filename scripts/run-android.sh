#!/bin/bash

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")/.."

pushd android
adb shell am force-stop com.webmr.exokit
adb logcat -c
adb shell am start -n com.webmr.exokit/android.app.NativeActivity
adb logcat
popd

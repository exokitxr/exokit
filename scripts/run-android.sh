#!/bin/bash

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")"

pushd android
adb shell am force-stop com.webmr.exokit
adb shell am start -n com.webmr.exokit/android.app.NativeActivity
adb logcat -c
adb logcat -d | less
popd

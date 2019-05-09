#!/bin/bash

# NOTE: you need this patch in $ANDROID_HOME/ndk-bundle/prebuilt/linux-x86_64/bin/ndk-gdb.py
#
#   def dump_var(args, variable, abi=None):
# +   return "arm64-v8a"
#
# Otherwise, you will get `ERROR: Failed to retrieve application ABI from Android.mk.`

# Initialization.

set -e

# Preface.

cd "$(dirname "$0")/.."

pushd ./android/app/src/main
"$ANDROID_HOME/ndk-bundle/ndk-gdb" --adb="$ANDROID_HOME/platform-tools/adb" --launch=android.app.NativeActivity
popd

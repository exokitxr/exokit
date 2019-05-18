#!/bin/bash

# NOTE: you need this patch in $ANDROID_HOME/ndk-bundle/prebuilt/linux-x86_64/bin/ndk-gdb.py
#
#   def dump_var(args, variable, abi=None):
# +   return "arm64-v8a"
#
# Otherwise, you will get `ERROR: Failed to retrieve application ABI from Android.mk.`

# NOTE: to add arguments, put this in $ANDROID_HOME/ndk-bundle/prebuilt/linux-x86_64/bin/ndk-gdb.py
#
# am_cmd.append("-e")
# am_cmd.append("ARGS")
# am_cmd.append("'node /package /package/examples/tutorial.html'")

set -e


cd "$(dirname "$0")"

source ./version-android.sh

pushd ../android/app/src/main
"$ANDROID_HOME/ndk-bundle/ndk-gdb" --adb="$ANDROID_HOME/platform-tools/adb" --launch=android.app.NativeActivity
popd

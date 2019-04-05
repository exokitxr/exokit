#!/bin/bash

# Initialization.

set -e

# Preface.

TOOLCHAIN="$PWD/../ndk"
cd "$(dirname "$0")"

pushd android
adb logcat -c
adb logcat -d | less
popd

#!/bin/bash

# Initialization.

set -e

# Make standalone toolchain 

$ANDROID_HOME/ndk-bundle/build/tools/make_standalone_toolchain.py --arch arm64 --api 26 --install-dir $ANDROID_HOME/ndk-bundle/toolchain

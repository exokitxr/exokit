#!/bin/bash

set -e

source ./version-ml.sh

cmd.exe /c "$MLSDK_WIN/debug.cmd" --deploy-mpk build/magicleap/exokit.mpk build/magicleap/program-device/release_lumin_clang-3.8_aarch64/program-device --env "ARGS=node . $@"

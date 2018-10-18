#!/bin/bash

export MLSDK=${MLSDK:-/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.16.0}
export MLSDK_WIN=$(echo "$MLSDK" | sed 's/^\/mnt\/\([a-z]\)\//\1:\\/' | sed 's/\//\\/g')

cmd.exe /c "$MLSDK_WIN/debug.cmd" --deploy-mpk build/magicleap/exokit.mpk build/magicleap/program-device/release_lumin_clang-3.8_aarch64/program-device --env "ARGS=node . $@"

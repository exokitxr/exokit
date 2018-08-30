#!/bin/bash

export MLSDK='/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.16.0'
export MLSDK_WIN='C:\Users\avaer\MagicLeap\mlsdk\v0.16.0'

cmd.exe /c "$MLSDK_WIN/tools/debug/debug.py" --deploy-mpk build/magicleap/exokit.mpk build/magicleap/program-device/release_lumin_clang-3.8_aarch64/program-device --env "ARGS=node . $@"
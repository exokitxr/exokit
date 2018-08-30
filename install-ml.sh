#!/bin/bash

export MLSDK=${MLSDK:-/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.16.0}
export MLSDK_WIN=$(echo "$MLSDK" | sed 's/^\/mnt\/c\//C:\\/' | sed 's/\//\\/g')

cmd.exe /c "$MLSDK_WIN/tools/mldb/mldb.exe" install -u build/magicleap/exokit.mpk
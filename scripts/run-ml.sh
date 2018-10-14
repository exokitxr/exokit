#!/bin/bash

export MLSDK=${MLSDK:-/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.16.0}
export MLSDK_WIN=$(echo "$MLSDK" | sed 's/^\/mnt\/\([a-z]\)\//\1:\\/' | sed 's/\//\\/g')

cmd.exe /c "$MLSDK_WIN/tools/mldb/mldb.exe" terminate -f com.webmr.exokit
cmd.exe /c "$MLSDK_WIN/tools/mldb/mldb.exe" launch com.webmr.exokit -v "ARGS=node . $@"
cmd.exe /c "$MLSDK_WIN/tools/mldb/mldb.exe" log exokit:*

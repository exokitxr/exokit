#!/bin/bash

export MLSDK=${MLSDK:-/mnt/c/Users/chris/MagicLeap/mlsdk/v0.19.0}
export MLSDK_WIN=$(echo "$MLSDK" | sed 's/^\/mnt\/\([a-z]\)\//\1:\\/' | sed 's/\//\\/g')

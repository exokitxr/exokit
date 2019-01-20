#!/bin/bash

set -e

cd "$(dirname "$0")"

source ./version-ml.sh

cmd.exe /c "$MLSDK_WIN/tools/mldb/mldb.exe" terminate -f com.webmr.exokit || true
cmd.exe /c "$MLSDK_WIN/tools/mldb/mldb.exe" launch com.webmr.exokit -v "ARGS=node --experimental-worker . $@"
cmd.exe /c "$MLSDK_WIN/tools/mldb/mldb.exe" log exokit:*

#!/bin/bash

set -e

cd "$(dirname "$0")"

source ./version-ml.sh

cmd.exe /c "$MLSDK_WIN/tools/mldb/mldb.exe" install -u ../../build/magicleap/exokit.mpk

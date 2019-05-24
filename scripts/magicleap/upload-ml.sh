#!/bin/bash

set -e

cd "$(dirname "$0")"

source ./version-ml.sh

"$MLSDK/tools/mldb/mldb.exe" -d install -u ../../build/magicleap/app-device/app-device.mpk

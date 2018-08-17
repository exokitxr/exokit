#!/bin/bash

export MLSDK='/mnt/c/Users/avaer/MagicLeap/mlsdk/v0.16.0'

"$MLSDK/tools/mldb/mldb.exe" -d install -u build/magicleap/app-device/app-device.mpk

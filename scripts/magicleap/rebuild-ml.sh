#!/bin/bash

set -e

rm -Rf node_modules
npm cache clean --force
rm -Rf build/magicleap

./scripts/magicleap/build-ml.sh "$@"

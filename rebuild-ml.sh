#!/bin/bash

rm -Rf node_modules
npm cache clean --force
rm -Rf build/magicleap

./build-ml.sh "$@"
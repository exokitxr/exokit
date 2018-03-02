#!/bin/bash

case "$OSTYPE" in
  linux*) ./node_modules/isolator/lib/linux/isolator -- node/node . "$@" ;;
  darwin*) ./node_modules/isolator/lib/macos/isolator -- node/node . "$@" ;; 
  *) echo "Operating system not supported: $OSTYPE" ;;
esac

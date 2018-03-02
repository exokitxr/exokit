#!/bin/bash

NODE_BIN="node"
if [ -e "node/node" ]; then
  NODE_BIN="node/node"
fi

case "$OSTYPE" in
  linux*) ./node_modules/isolator/lib/linux/isolator -- "$NODE_BIN" . "$@" ;;
  darwin*) ./node_modules/isolator/lib/macos/isolator -- "$NODE_BIN" . "$@" ;;
  *) echo "Operating system not supported: $OSTYPE" ;;
esac

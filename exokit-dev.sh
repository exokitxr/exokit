#!/bin/bash

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

NODE_BIN="node"
if [ -x "$DIR/node/bin/node" ]; then
  NODE_BIN="$DIR/node/bin/node"
fi

case "$OSTYPE" in
  linux*) ./node_modules/isolator/lib/linux/isolator -- "$NODE_BIN" "$DIR/index.js" "$@" ;;
  darwin*) ./node_modules/isolator/lib/macos/isolator -- "$NODE_BIN" "$DIR/index.js" "$@" ;;
  *) echo "Operating system not supported: $OSTYPE" ;;
esac

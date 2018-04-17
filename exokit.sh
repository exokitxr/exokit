#!/bin/sh

# resolve symlinks
dir="$(pwd)"
bin="$0"
while [ -L "${bin}" ]
do
    x="$(readlink "${bin}")"
    cd "$(dirname "${bin}")"
    bin="${x}"
done
cd "$(dirname "${bin}")"
home="$(pwd)"
cd "${dir}"

# find node path
NODE_BIN="node"
if [ -e "node/node" ]; then
  NODE_BIN="node/node"
fi

# execute
exec "$NODE_BIN" index.js "$@"

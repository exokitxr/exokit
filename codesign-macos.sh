#!/bin/sh
set -x
find /tmp/Exokit.app -type f | xargs codesign --force --verify --verbose --sign "7C22D41BA5AB743D3E47D543F6B27FE2FC720412"
codesign --force --verify --verbose --sign "7C22D41BA5AB743D3E47D543F6B27FE2FC720412" /tmp/Exokit.app

#!/bin/sh
set -x

if [ ! -z "${PFX_KEY}" ]
then
  security create-keychain -p nopassword build.keychain
  security default-keychain -s build.keychain
  security unlock-keychain -p nopassword build.keychain
  security set-keychain-settings -t 3600 -u build.keychain
  security import codesign-macos.pfx -k build.keychain -P "${PFX_KEY}" -A
  security set-key-partition-list -S apple-tool:,apple: -s -k nopassword build.keychain
  find /tmp/Exokit.app -type f | xargs codesign --force --verify --verbose --sign "7C22D41BA5AB743D3E47D543F6B27FE2FC720412"
  codesign --force --verify --verbose --sign "7C22D41BA5AB743D3E47D543F6B27FE2FC720412" /tmp/Exokit.app
  cp appdmg-codesign.json /tmp
  appdmg /tmp/appdmg-codesign.json exokit-macos-x64.dmg
else
  cp appdmg.json /tmp
  appdmg /tmp/appdmg.json exokit-macos-x64.dmg
fi

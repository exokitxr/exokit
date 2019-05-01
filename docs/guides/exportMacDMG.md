---
title: Export XR site to macOS DMG
type: guides
layout: docs
order: 4
parent_section: guides
---

This guide explains how to package your XR site with the Exokit Engine as an DMG application for macOS.

## Set default site
In the Exokit [`scripts/exokit-macos`](https://github.com/exokitxr/exokit/blob/master/scripts/exokit-macos#L17) is where you will find the default site that the Exokit DMG application will load into. Change the argument passed in to your site URL or file path.

## Prerequisites
Latest Exokit

## Build dmg
- npm install -g appdmg
 - curl "https://nodejs.org/dist/v11.6.0/node-v11.6.0-darwin-x64.tar.gz" >node.tar.gz
 - tar -zxf node.tar.gz
 - rm node.tar.gz
 - mv node-v11.6.0-darwin-x64 node
 - export PATH="$(pwd)/node/bin:$PATH"
 - unset NVM_NODEJS_ORG_MIRROR
 - "./node/bin/npm install --no-optional"
 - export TEST_ENV=ci
 - "./node/bin/npm run test"
 - install_name_tool -change '@rpath/OpenVR.framework/Versions/A/OpenVR' '@loader_path/../../node_modules/native-openvr-deps/bin/osx64/OpenVR.framework/Versions/A/OpenVR'
   build/Release/exokit.node
 - mkdir -p /tmp/Exokit.app/Contents/MacOS
 - cp -R * /tmp/Exokit.app/Contents/MacOS
 - mkdir -p /tmp/Exokit.app/Contents/Resources
 - cp -R metadata/icon.icns /tmp/Exokit.app/Contents/Resources
 - cp metadata/Info.plist /tmp/Exokit.app/Contents
 - rm -R node
- ./scripts/exokit-codesign-macos.sh

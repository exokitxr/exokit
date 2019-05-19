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
Node `11.6.0`
XCode
Latest Exokit

## Build DMG

Navigate to the Exokit directory
```sh
cd exokit
```

Install [appdmg](https://github.com/LinusU/node-appdmg)
```sh
npm install -g appdmg
```

Install and extract the necessary Node.js version
```sh
curl "https://nodejs.org/dist/v11.6.0/node-v11.6.0-darwin-x64.tar.gz" >node.tar.gz
tar -zxf node.tar.gz
rm node.tar.gz
mv node-v11.6.0-darwin-x64 node
```

Export Node.js path and build Exokit module
```sh
export PATH="$(pwd)/node/bin:$PATH"
unset NVM_NODEJS_ORG_MIRROR
"./node/bin/npm install --no-optional"
install_name_tool -change '@rpath/OpenVR.framework/Versions/A/OpenVR' '@loader_path/../../node_modules/native-openvr-deps/bin/osx64/OpenVR.framework/Versions/A/OpenVR' build/Release/exokit.node
```

Copy all Exokit files to a temp directory
```sh
mkdir -p /tmp/Exokit.app/Contents/MacOS
cp -R * /tmp/Exokit.app/Contents/MacOS
```

Copy Exokit metadata to appropriate directories
```sh
mkdir -p /tmp/Exokit.app/Contents/Resources
cp -R metadata/icon.icns /tmp/Exokit.app/Contents/Resources
cp metadata/Info.plist /tmp/Exokit.app/Contents
```

Run build script
```sh
./scripts/macos/exokit-codesign-macos.sh
```

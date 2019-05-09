---
title: Export XR site to Windows EXE
type: guides
layout: docs
order: 3
parent_section: guides
---

This guide explains how to package your XR site with the Exokit Engine as an EXE for Windows.

## Set default site
In the Exokit [`scripts/exokit.sh`](https://github.com/exokitxr/exokit/blob/master/scripts/exokit.sh#L17) is where you will find the default site that the Exokit EXE application will load into. Change the argument passed in to your site URL or file path.

## Prerequisites
Node `11.6.0`
Latest Exokit
Python 2
Visual Studio 2015 or 2017

Alternative requirements: Node.js 11.6.0, as administrator: `npm install -g windows-build-tools`

## Build EXE

Install and extract the necessary Node.js version
```sh
wget "https://nodejs.org/dist/v11.6.0/node-v11.6.0-win-x64.zip" -OutFile node.zip
7z x node.zip
rm node.zip
mv node-v11.6.0-win-x64 node
```

Export Node.js path and build Exokit module
```sh
$env:Path = "$pwd\node;$env:Path";
.\node\npm install --no-optional
```

Build EXE to `dist` directory
```sh
mkdir dist
.\buildtools\iscc "$pwd\metadata\exokit.iss" "/dMyAppVersion=$version" /odist /qp
mv dist\*.exe exokit-win-x64.exe
```

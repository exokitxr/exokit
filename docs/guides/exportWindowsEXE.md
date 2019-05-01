---
title: Export XR site to Windows EXE
type: guides
layout: docs
order: 3
parent_section: guides
---

This guide explains how to package your XR site with the Exokit Engine as an MPK for the Magic Leap One headset.

## Set default site
In the Exokit [`scripts/exokit.sh`](https://github.com/exokitxr/exokit/blob/master/scripts/exokit.sh#L17) is where you will find the default site that the Exokit EXE application will load into. Change the argument passed in to your site URL or file path.

## Prerequisites
Latest Exokit

## Build exe
wget "https://nodejs.org/dist/v11.6.0/node-v11.6.0-win-x64.zip" -OutFile node.zip
7z x node.zip
rm node.zip
mv node-v11.6.0-win-x64 node
$env:Path = "$pwd\node;$env:Path";
.\node\npm install --no-optional
$env:TEST_ENV = 'ci'
.\node\npm run test
# bash scripts/testRun.sh
set version "$env:APPVEYOR_REPO_TAG_NAME"
if ([string]::IsNullOrEmpty("$version")) { set version "$env:APPVEYOR_REPO_COMMIT".Substring(0, 8) }
mkdir dist
.\buildtools\iscc "$pwd\metadata\exokit.iss" "/dMyAppVersion=$version" /odist /qp
mv dist\*.exe exokit-win-x64.exe

---
title: Export XR site to Magic Leap One MPK 
type: guides
layout: docs
order: 2 
parent_section: guides 
---

This guide explains how to package your XR site with the Exokit Engine as an MPK for the Magic Leap One headset.

## Set default site
In the Exokit [main.cpp](https://github.com/exokitxr/exokit/blob/f10dadf0013de0a35a5e72046140a0345987ab80/main.cpp#L416) is where you will find the default site that the Exokit MPK will load into. Change the `jsString` to your site URL or file path.

## Prerequisites
Node `11.6.0`
Latest Exokit

## Install the Magic Leap SDK

Sign in as a Magic Leap creator:
https://www.magicleap.com/creator

Download the Package Manager:
https://creator.magicleap.com/downloads/lumin-sdk/overview

Once the package manager is opened, download the 0.19.0 version of the SDK.

## Build MPK

Confirm that `MLSDK` in `version-ml.sh` points to the Magic Leap SDK directory
```sh
cd exokit
# edit MLSDK path in ./scripts/version-ml.sh
```

Build the MPK
```sh
./scripts/build-ml.sh
```

If you want to now install and run into your plugged in device using mldb
```sh
./scripts/install-ml.sh
./scripts/run-ml.sh
```


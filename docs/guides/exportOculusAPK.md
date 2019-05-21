---
title: Export XR site to Oculus Mobile APK
section_title: Guides
type: guides
layout: docs
parent_section: docs
order: 1
section_order: 2
---

This guide explains how to package your XR site with the Exokit Engine as an APK for Oculus Mobile (Oculus Go, Oculus Quest) devices.

### Set default site
In the Exokit [`main.cpp`](https://github.com/exokitxr/exokit/blob/f10dadf0013de0a35a5e72046140a0345987ab80/main.cpp#L416) is where you will find the default site that the Exokit APK will load into. Change the `jsString` to your site URL or file path.

## OPTION 1- Docker
The upside of docker is that it requires minimal environment setup. The downside is that it will do _every_ instruction _everytime_ it is run.

Building an APK with the Dockerfile is as simple as:

- `docker pull debian:latest`
- `docker build -f android.Dockerfile -t exokit .`
- `docker run exokit cat exokit.apk >exokit.apk`


## OPTION 2- Manual environment setup

### Prerequisites
Node `11.6.0`  
Latest Exokit  
Ubuntu Windows Subsystem for Linux (WSL)


### Install Android sdkmanager

Instructions for getting the Android SDK manager, adb, ndk-bundle, etc:


To install it on a Debian-based system, simply do:
```sh
# Install latest JDK
sudo apt install default-jdk

# install unzip if not installed yet
sudo apt install unzip

# get latest sdk tools - link will change. go to https://developer.android.com/studio/#downloads to get the latest one
cd ~
wget https://dl.google.com/android/repository/sdk-tools-linux-4333796.zip

# unpack archive
unzip sdk-tools-linux-4333796.zip

rm sdk-tools-linux-4333796.zip

mkdir android-sdk
mv tools android-sdk/tools
```

Then add the Android SDK to your PATH: open ~/.bashrc in editor and add the following lines into the file:
```
# Export the Android SDK path
export ANDROID_HOME=$HOME/android-sdk
export PATH=$PATH:$ANDROID_HOME/tools/bin
export PATH=$PATH:$ANDROID_HOME/platform-tools

# Fixes sdkmanager error with java versions higher than java 8
export JAVA_OPTS='-XX:+IgnoreUnrecognizedVMOptions --add-modules java.se.ee'
```

Run:

`source ~/.bashrc`

Show all available SDK packages:

`sdkmanager --list`

Accept all sdkmanager licenses:

`yes | $ANDROID_HOME/tools/bin/sdkmanager --licenses`

Identify latest android platform (here it's 28) and run:

`sdkmanager "platform-tools" "platforms;android-28"`

Now you have adb, fastboot and the latest sdk tools installed.

To install NDK directly, run:

`sdkmanager "ndk-bundle"`

### Build APK

Confirm that `$ANDROID_HOME` points to Android/SDK directory

Make the NDK standalone toolchain:
```sh
cd exokit
./scripts/make-toolchain-android.sh
```

Build the APK:
```sh
# Run this inside Ubuntu WSL
./scripts/build-android.sh
```

If you want to now install and run in your plugged-in device using adb:
```sh
./scripts/install-android.sh
./scripts/run-android.sh
```

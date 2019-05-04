---
title: Build from Source
section_title: SDK
type: sdk
layout: docs
order: 0
parent_section: docs
section_order: 3
---

The main project repository is at [`exokitxr/exokit`](https://github.com/exokitxr/exokit). You can clone it the regular way:

```sh
git clone https://github.com/exokitxr/exokit.git
```


## Install and build

Make sure to be using Node `v11.6.0`. We recommend using [nvm](https://github.com/nvm-sh/nvm) or [nvm-windows](https://github.com/coreybutler/nvm-windows) for managing Node versions.
After Node `v11.6.0` is installed, all of the Exokit dependencies and native code can be build with `npm install`:

```sh
cd exokit
npm install
```

### Linux dependencies

Linux additionally requires that you install some local dependencies. For `debian`/`ubuntu` they are:
```sh
apt-get install -y \
build-essential wget python libglfw3-dev libglew-dev libfreetype6-dev libfontconfig1-dev uuid-dev libxcursor-dev libxinerama-dev libxi-dev libasound2-dev libexpat1-dev
```

## Common build errors
This section covers common errors when building the Exokit engine and their solutions.

### `Error: Cannot find module './build/Release/vm_one.node'`

There was a build failure that was ignored and `node_modules` contains the failure. Wipe `node_modules` and try to build again.

```sh
rm -rf node_modules
npm cache clean
```

### `<SkImage.h> not found`, or a missing header

This means a prebuilt dependency did not extract. Clear your `npm`/`yarn` cache, `rm -Rf node_modules` and `npm install` again.

#### Notes

1. Exokit's bindings code is a native module build using the [`binding.gyp` recipe](https://github.com/exokitxr/exokit/blob/master/binding.gyp).
1. Exokit downloads and uses several prebuilt dependencies. They have been pre-compiled for all supported architectures and uploaded to npm. They self-extract themselves based off detected architecture.

## Run Exokit Engine

Exokit Engine is just a node module, so you can run it with node.
From the `exokit` git clone directory, do:

```sh
node . <site URL or file path>
```

## Debugging

You can use the regular Node debugging tools and gdb to debug Exokit Engine. See the [debugging documentation](debugging.md) for more information.

## Contributing

Please [file an issue](https://github.com/exokitxr/exokit/issues) if you encounter any bugs. Pull Requests are very much welcome as well.

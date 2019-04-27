---
title: Start Development
type: sdk
layout: docs
order: 1
parent_section: sdk
---

The main project repository is at [`exokitxr/exokit`](https://github.com/exokitxr/exokit). You can clone it the regular way:

```sh
git clone https://github.com/exokitxr/exokit.git
```


## Install and build

All of the Exokit depedencies and native code can be build with `npm install`:

```sh
npm install
```

## Linux dependencies

Linux additionally requires that you install some local dependencies. For `debian`/`ubuntu` they are:
```sh
apt-get install -y \
build-essential wget python libglfw3-dev libglew-dev libfreetype6-dev libfontconfig1-dev uuid-dev libxcursor-dev libxinerama-dev libxi-dev libasound2-dev libexpat1-dev
```

#### Notes

1. Exokit's bindings code is a native module build using the [`binding.gyp` recipe](https://github.com/exokitxr/exokit/blob/master/binding.gyp).
1. Exokit downloads and uses several prebuilt dependencies. They have been pre-compiled for all supported architectures and uploaded to npm. They self-extract themselves based off detected architecture.

# Run Exokit Engine

> Exokit Engine is just a node module, so you can run it with node.

From the `exokit` git clone directory, do:

```sh
node .
```

This will start the Exokit Engine CLI.

```sh
[x] // enter code here
```

Here, you can enter:

- A URL, which will trigger navigation
- `<dom>` elements
- `dom = <assignment>` expressions
- Commands to run in the context of the `window`

> Note: This is the same experience as you get with the Exokit installer when running `Exokit Engine CLI`.

## Logging

Exokit has a logging mode enabled by the `-l` flag:

```sh
exokit -l https://aframe.io/a-painter/ 
```

The logs go in `$HOME/.exokit/log.txt`. For window that generally means `C:\username\.exokit\log.txt`.

## Debugging

You can use the regular Node debugging tools to debug Exokit Engine. See the [debugging doc](debugging.md) for more information.

## Contributing

See something? Please do [file an issue](https://github.com/exokitxr/exokit/issues)!

Pull Requests are very much welcome as well.

#### Troubleshooting

See the [common build errors](commonBuildErrors.md) documentation for more info.

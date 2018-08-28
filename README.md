# Exokit
### Javascript post-screen AR/VR/WebXR engine  🦖

<img src="https://github.com/webmixedreality/exokit/blob/master/icon.png" width=100/>

[![Slack](https://exoslack.now.sh/badge.svg)](https://communityinviter.com/apps/exokit/exokit)
[![Github releases](https://img.shields.io/github/downloads/webmixedreality/exokit/total.svg)](https://github.com/webmixedreality/exokit/releases )
[![npm package](https://img.shields.io/npm/v/exokit.svg)](https://www.npmjs.com/package/exokit)
[![Travis CI build status](https://travis-ci.org/modulesio/exokit-windows.svg?branch=master)](https://travis-ci.org/modulesio/exokit-windows)
[![Appveyor build status](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?svg=true)](https://ci.appveyor.com/project/modulesio/exokit-windows)
[![Twitter Follow](https://img.shields.io/twitter/follow/webmixedreality.svg?style=social)](https://twitter.com/webmixedreality)

# Quickstart

#### [Download for current OS](https://get.webmr.io)

#### Build your own

```
git clone https://github.com/webmixedreality/exokit.git
cd exokit
npm install
node . # or node -h for home
```

#### Run a WebGL/WebXR site

```
exokit https://emukit.webmr.io/ # start Emukit in Exokit
```

Subscribe to our Email list to be notified when we launch! http://eepurl.com/dFiLMz

## Introduction

<img src="assets/screenshots.gif" alt="Screenshots"/>

### Exokit is a post-screen web engine.

It loads regular HTML5 pages using standards like:
- WebGL
- WebXR
- WebAudio
- Etc.

It's designed for WebGL, WebGL2, WebXR, and immersive AR/VR. It's faster than Chrome, extensible and embeddable into other environments.

The catch? _It doesn't do 2D. (But it comes with a build of Chromium, which does.)_

It can open blend multiple WebXR sites at a time with "reality tabs".

Exokit is agnostic about how you write your code. It works with frameworks like [THREE.js](https://github.com/mrdoob/three.js/), [A-Frame](https://aframe.io/), [Babylon.js](https://github.com/BabylonJS/Babylon.js), and web builds of [Unity](https://unity3d.com).

## Architecture

Exokit is a Javascript [Node.js](https://nodejs.org) module.

Lightweight C++ bindings hook into WebGL, WebXR, Magic Leap, Leap Motion, and various other device APIs.

It's also extensible and embeddable -- you can add your own things to the browser core, and `const {window} = require('exokit')()` to get an immersive browser in another project.

Exokit runs on Windows, macOS, Linux (x64), and Linux (ARM64).

## Web API support

- HTTP/S
- HTML5
- `<script>`
- DOM
- WebGL
- WebXL
- Canvas2D
- WebSocket
- Web Workers
- `<img>`, `<audio>`, `<video>`
- WebAudio
- Keyboard/Mouse events
- Gamepad API
- `<iframe>`
- **No** HTML layout
- **No** HTML rendering
- **No** CSS
- **No** Legacy APIs

## Hardware bindings

- OpenGL
- OpenVR (Steam VR)
- Magic Leap
- Leap Motion

## Why Exokit?

- You want your WebGL/WebXR to run fast.
- You want the hot new web APIs.
- You want to add your own integrations -- including native -- into a browser environment.
- You want a lightweight browser as a hackable node module.
- You want to combine the web with a 3D engine like Unity.

## Why not Exokit?

- You're looking for a "web browser".
- You don't care about 3D or mixed reality.
- You're looking for strict and/or legacy standards support.

## Debugging

Uses [ndb](https://github.com/GoogleChromeLabs/ndb).

```js
npm run debug
```

Then in the console, input:

```
let window = await require('./index').load(yourUrl);
```

Now you have a handle on the window object as you test your application, and
you can set `debugger` breakpoints and such.

## Keyboard

![Keyboard](https://raw.githubusercontent.com/webmixedreality/exokit/master/assets/keyboard.png)

## Community

- [Slack](https://communityinviter.com/apps/exokit/exokit) for development
- [Discord](https://discord.gg/Apk6cZN) for hanging out
- [Twitter](https://twitter.com/webmixedreality) for silliness

## Contributors

- [Avaer K](//github.com/modulesio)
- [Nick L](//github.com/Fierent)
- [Chris Eddy](//github.com/ChrisEddy)
- [Chris La Torres](https://github.com/chrislatorres)
- [Noah A S](//github.com/NoahSchiffman)

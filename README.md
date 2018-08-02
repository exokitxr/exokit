# Exokit Engine
### Post-screen web engine for AR/VR, written in Javascript 🦖

[![Slack](https://exoslack.now.sh/badge.svg)](https://exoslack.now.sh)
[![Github releases](https://img.shields.io/github/downloads/webmixedreality/exokit/total.svg)](https://github.com/webmixedreality/exokit/releases )
[![npm package](https://img.shields.io/npm/v/exokit.svg)](https://www.npmjs.com/package/exokit)
[![Travis CI build status](https://travis-ci.org/modulesio/exokit-windows.svg?branch=master)](https://travis-ci.org/modulesio/exokit-windows)
[![Appveyor build status](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?svg=true)](https://ci.appveyor.com/project/modulesio/exokit-windows)
[![Twitter Follow](https://img.shields.io/twitter/follow/webmixedreality.svg?style=social)](https://twitter.com/webmixedreality)

# Quickstart

#### [Download for current OS](https://get.webmr.io)

#### Or build your own

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

## Introduction

<p align="center">
   <a href="https://google.com/">
    <img src="http://via.placeholder.com/218x218" alt="Architecture diagram"/>
  </a>
<a href="https://google.com/">
    <img src="http://via.placeholder.com/218x218" alt="Exokit flips the browser inside-out in order to be fast"/>
  </a>
  <a href="https://google.com/">
    <img src="http://via.placeholder.com/218x218" alt="In most browsers 3D is a footnote to 2D, but not in Exokit"/>
  </a>
  <a href="https://google.com/">
    <img src="http://via.placeholder.com/218x218" alt="Not a fork of Chrome/Firefox"/>
  </a>
</p>

**Exokit is a post-screen web engine.**

_It's for WebGL, WebGL2, WebXR, and immersive AR/VR. It's fast, extensible and embeddable. It doesn't do 2D._

_It can open multiple WebXR sites at a time and blend them with reality tabs._

Exokit engine loads regular HTML5 pages using standards like WebGL, WebXR, WebAudio. It works with your favorite engines and frameworks, like [THREE.js](https://github.com/mrdoob/three.js/), [A-Frame](https://aframe.io/), [Babylon.js](https://github.com/BabylonJS/Babylon.js), and even web builds of [Unity](https://unity3d.com).

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

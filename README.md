# Exokit Browser
### Javascript AR/VR web browser 🦖

[![Slack](https://exoslack.now.sh/badge.svg)](https://exoslack.now.sh)
[![Github releases](https://img.shields.io/github/downloads/webmixedreality/exokit/total.svg)](https://github.com/webmixedreality/exokit/releases )
[![npm package](https://img.shields.io/npm/v/exokit.svg)](https://www.npmjs.com/package/exokit)
[![Travis CI build status](https://travis-ci.org/modulesio/exokit-windows.svg?branch=master)](https://travis-ci.org/modulesio/exokit-windows)
[![Appveyor build status](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?svg=true)](https://ci.appveyor.com/project/modulesio/exokit-windows)
[![Twitter Follow](https://img.shields.io/twitter/follow/webmixedreality.svg?style=social)](https://twitter.com/webmixedreality)

# Quickstart

[Download for current OS](https://get.webmr.io)

#### Run a WebGL/WebXR site:
```
exokit https://emukit.webmr.io/ # start Emukit in Exokit
```

<img src="http://via.placeholder.com/600x400"/>

## Introduction

<p align="center">
   <a href="https://google.com/">
    <img src="http://via.placeholder.com/300x300" alt="Architecture diagram"/>
  </a>
  <a href="https://google.com/">
    <img src="http://via.placeholder.com/300x300" alt="In most browsers 3D is a footnote to 2D"/>
  </a>
  <a href="https://google.com/">
    <img src="http://via.placeholder.com/300x300" alt="Not a fork of Chrome/Firefox"/>
  </a>
</p>

[Exokit Browser](https://exokit.webmr.io) is a post-screen web engine. It doesn't do 2D -- it's for WebGL(2), WebXR, and immersive AR/VR. It's fast, extensible and embeddable. It can open multiple WebXR sites at a time and blend them with Reality Tabs.

Exokit engine loads regular HTML5 pages using standards like WebGL, WebXR, WebAudio. It works with your favorite engines and frameworks, like [THREE.js](https://github.com/mrdoob/three.js/), [A-Frame](https://aframe.io/), [Babylon.js](https://github.com/BabylonJS/Babylon.js), and even Web builds of [Unity](https://unity3d.com).

In terms of architecture, Exokit is a Javascript Node module. Simple C++ bindings hook into WebGL, WebXR, Magic Leap, Leap Motion, and various other device APIs. It's also extensible and embeddable -- you can add your own things to the browser core, and `require('exokit')` to get an immersive browser in another project.

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

## Additional bindings

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

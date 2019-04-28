---
title: Introduction
section_title: Introduction
type: introduction
layout: docs
order: 0
parent_section: docs
section_order: 1
installation: true
examples:
---

## Overview

Exokit is an **engine that runs XR experiences made with regular web code**. It emulates a web browser on top of Node.js, providing native hooks for WebGL, WebXR, WebAudio, and other standard APIs for immersive experiences. It works with web pages and your favorite frameworks like A-Frame.

:eyeglasses: **Exokit currently targets the following platforms**:
* OpenVR Desktop VR (Steam compatible)
* Oculus Desktop (Oculus Rift/Rift S)
* Oculus Mobile (Oculus Quest/Go, GearVR)
* Magic Leap
* iOS ARKit *
* Android ARCore *
* Google VR (Daydream / Cardboard / Mirage Solo) *
* any XR device, start a [pull request](https://github.com/exokitxr/exokit/compare) with a native binding if it isn't listed here! *

\* not supported yet

:electric_plug: **Since Exokit supports anything that runs on the web, it powers experiences built with**:
* Three.js
* Unity
* Pixi.js
* Babylon.js
* A-Frame
* Custom WebGL frameworks
* WebAssembly, TypeScript, and any language that transpiles to JavaScript

:thumbsup: **Why Exokit?**

- You want your WebGL/WebXR to run fast.
- You want the hot new web APIs.
- You want to add your own integrations -- including native -- into a browser environment.
- You want a lightweight browser as a hackable node module.
- You want to combine the web with a 3D engine like Unity.

:thumbsdown: **Why not Exokit?**

- You're looking for a traditional "web browser".
- You don't care about 3D or mixed reality.
- You're looking for strict and/or legacy standards support.

:book: **Manifesto**

- The future is immersive. The web is the best application platform. Javascript is the best ecosystem.
- Content should be hardware agnostic. Tomorrow will have different hardware. VR and AR should be compatible.
- It's not possible to do both 2D and 3D well. We don't do 2D. We can use an external 2D browser.
- Use your favorite game engine. Exokit is not a game engine.
- Legacy browser design choices don't make sense in XR.
- Exokit empowers and connect apps, even (especially) if they aren't designed to cooperate.
- Apps should run in "reality tabs", layers of reality that blend together.

## Examples

<a href="https://youtu.be/cd_DEwCDF6U"><img alt="Hands Reality Tab" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507781-0e463300-561e-11e9-9b1a-f43b8259d041.gif" height="190" width="32%"></a>
<a href="https://youtu.be/b-UKSg0QCRE"><img alt="Live Reload Magic Leap" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507118-a216ff80-561c-11e9-829e-74d8244571c3.gif" height="190" width="32%"></a>
<a href="https://youtu.be/O1xA1r5SZUM"><img alt="Tutorial Reality Tab" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507125-a3e0c300-561c-11e9-835f-3a26a9e879b5.gif" height="190" width="32%"></a>

<a href="https://www.youtube.com/watch?v=m_QntqZmd_Q"><img alt="Reality Projection with HTC Vive and Magic Leap" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507271-e60a0480-561c-11e9-87ad-7dc736ba0760.gif" height="190" width="32%"></a>
<a href="https://youtu.be/i0MXRCNkdB4"><img alt="Emukit" target="_blank" src="https://user-images.githubusercontent.com/29695350/55507623-a8f24200-561d-11e9-97a3-194b6b4a1d8b.gif" height="190" width="32%"></a>
<a href="https://exokit.org/"><img alt="Various Exokit Apps" target="_blank" src="https://user-images.githubusercontent.com/29695350/55506701-ba3a4f00-561b-11e9-9e19-ba808bed7c5a.gif" height="190" width="32%"></a>

*Find more examples at [the showcase](../../showcase), [on GitHub](https://github.com/exokitxr/exokit/tree/master/examples), and on [YouTube](https://www.youtube.com/channel/UC87Q7_5ooY8FSLwOec52ZPQ).*


## Getting Started

### Desktop
<h4><a href="https://get.exokit.org">Download for current OS</a></h4>

#### Run a WebXR site (desktop)

```sh
exokit https://aframe.io/a-painter/ # start A-Painter in Exokit
```

### Magic Leap

<h4><a href="https://get.exokit.org/magicleap">Download for Magic Leap</a></h4>

#### Run (Magic Leap device)

```sh
mldb connect 192.168.0.10:1131 # mldb needs to be connected; see MLDB documentation
mldb install -u exokit.mpk # downloaded or built package
mldb launch com.webmr.exokit -v "ARGS=node --experimental-worker . file:///package/examples/hello_ml.html" # or URL to load
mldb log exokit:*
```

### Local Development

```sh
git clone https://github.com/exokitxr/exokit.git
cd exokit
npm install
node . <url> # or node . -h for home
```


[Installation]: ./installation.md

The [Installation] page provides more options for getting started with Exokit.


## Architecture

Exokit is a Javascript [Node.js](https://nodejs.org) module.

C++ bindings hook into WebGL, WebXR, Magic Leap, Leap Motion, and various other device APIs.

`const {window} = require('exokit')()` to get an immersive browser in another project.

Exokit runs on Windows, macOS, Linux (x64), Linux (ARM64), and Magic Leap (ARM64).

## Web API support

- HTTP/S
- HTML5
- `<script>`
- DOM
- WebGL
- WebXR
- WebVR
- WebRTC
- Canvas2D
- WebSocket
- Web Workers
- `<img>`, `<audio>`, `<video>`
- WebAudio
- Keyboard/Mouse events
- Gamepad API
- `<iframe>`
- ~~HTML layout~~
- ~~HTML rendering~~
- ~~CSS~~
- ~~Legacy APIs~~

## Hardware bindings

- OpenGL
- OpenVR (Steam VR)
- Oculus
- Oculus Mobile
- Magic Leap
- Leap Motion

## License

This program is free software and is distributed under an [MIT License](LICENSE.md).


## Off You Go!

If it's your first time here, here's a plan for success for getting into
Exokit:

1. For inspiration, check out what other people have built with Exokit on the
[Weekly Blog](https://exokit.org/blog/) or [WebVR Directory](https://webvr.directory).

2. Read through the documentation to get a grasp.

3. Stay in Touch:
- [Join our Discord](https://discord.gg/Apk6cZN) for hanging out.
- [Follow @exokitxr on Twitter](https://twitter.com/exokitxr) for updates.

4. Get involved! Check out the [Contributing Guide](https://github.com/exokitxr/exokit/CONTRIBUTING.md) for how to get started.

And it really helps to have dig into the fundamentals on JavaScript and
three.js. Have fun!

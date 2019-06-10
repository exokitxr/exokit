---
title: FAQ
type: introduction
layout: docs
parent_section: introduction
order: 14
---

A few of commonly asked Exokit questions.

## What is Exokit?  
Exokit is an XR web engine.

## How was Exokit started?
Exokit was created by Avaer Kazmer after creating Zeo, a WebVR minecraft clone. Avaer recognzied problems that weren't be solved and decided to start building an engine that would solve these problems.

## What is the philosophy behind Exokit?
The future is immerisve, content should be hardware agnostic, it is not possible to do 3D well while focusing on legacy 2D design.

## What is WebVR/WebXR?
The [WebXR Device API](https://github.com/immersive-web/webxr/blob/master/explainer.md) provides access to input and output capabilities commonly associated with Virtual Reality (VR) and Augmented Reality (AR) devices. It allows you develop and host VR and AR experiences on the web.

## What frameworks does Exokit support?
The Exokit Engine aspires to support any 3d web framework. Currently Exokit supports common frameworks and libraries such as THREE.js, A-Frame, Babylon.js. If your framework isn't currently supported, start a pull request or open an issue.

## What devices/platforms can Exokit export to?
The Exokit Engine aspires to export natively to any XR device. Currently Exokit can export to a variety of Standalone VR, Standalone Augmneted Reality, Desktop VR, Mobile devices such as: Oculus Quest, Oculus Go, HTC Vive, Oculus Rift, Oculus Rift S.

## What is the architecture of Exokit?
The Exokit Engine is a node module. The core is Javascript, which fetches HTML, runs `<script>` tags, and provides all of the regular XR web APIs like WebGL/WebXR/WebAudio.

The hardware-dependent parts are implemented as native bindings to Javascript. Exokit Engine uses GLFW for contexts, OpenGL for graphics, OpenVR for VR, ARCore for mobile AR, Leap Motion for hands detection, and a lot more, check out our [Integrations page](../overview/techIntegrations.html) for a more complete list.

## What web APIs does Exokit support?
Some of the [Web APIs](../apis/webAPIs) that the Exokit Engine supports are: WebVR, WebXR, THREE.js, websockets, web workers, fetch, HTML, express, react.

## What native APIs does Exokit support?
Some of the [Native APIs](../apis/nativeAPIs) that the Exokit Engine supports are: WebGL, Canvas2D, WebAudio, WebRTC, Node, OpenVR, oVR, Magic Leap, Leap Motion, Oculus Mobile, Chromium.

## What are reality layers?
Reality Layers are [`<iframe>`](../apis/iframeAPI)'s with added ability to do volumetric manipulation.

## Can reality layers interact with eachother?
Yes, reality layers can interact with eachother via `postMessage`.

## How can I contribute to Exokit?
The easiest way to get started is to clone the [Exokit repo](https://github.com/exokitxr/exokit) and then find [an issue](https://github.com/exokitxr/exokit/issues) that you want to take on.

## How can I report bugs or issues to Exokit?
Report bugs or issues by [opening an issue on GitHub](https://github.com/exokitxr/exokit/issues/new/choose).

## How can I download Exokit?
You can find download links on the [home page](https://exokit.org/).

## How can I start developing on Exokit?
The easiest way to get started is to clone the [Exokit repo](https://github.com/exokitxr/exokit) and then load into an XR site.

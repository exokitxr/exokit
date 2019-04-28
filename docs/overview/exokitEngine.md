---
title: Exokit Engine
section_title: Overview
type: overview
layout: docs
parent_section: docs
order: 0
section_order: 1
source_code:
examples: []
---

## Exokit is a native VR and AR Engine for JavaScript.

This project enables developers to build XR experiences using the same code that runs on the web. Exokit engine is written on top of Node and emulates a web browser, providing native hooks for WebGL, WebXR, WebAudio, and other APIs used in immersive experiences.


:eyeglasses: **Exokit currently targets the following platforms**:
* OpenVR Desktop VR (Steam compatible)
* Oculus Desktop (Oculus Rift/Rift S)
* Oculus Mobile (Oculus Quest/Go, GearVR)
* Magic Leap
* iOS ARKit *
* Android ARCore *
* Google VR (Daydream / Cardboard / Mirage Solo) *
* any XR device, start a [pull request](https://github.com/exokitxr/exokit/compare) with a native binding if it isn't listed here! *

### Since Exokit supports anything that runs on the web, it powers experiences built with:
- Three.js
- Unity
- Pixi.js
- Babylon.js
- AFrame
- Custom WebGL frameworks
- WebAssembly, TypeScript, and any language that transpiles to JavaScript.

## Subscribe to our Email list to be notified when we launch! http://eepurl.com/dFiLMz

> Exokit Engine is an HTML/Javascript engine that runs regular web sites, but only supports 3D via WebGL/WebXR.
> A window in Exokit is just the contents of a WebGL `<canvas>`.

Exokit Engine is primarily intended for AR/VR/Mixed Reality applications. It is _not_ based on another browser, specifically it is not a fork of Blink, Gecko, Webkit, or Trident.

<img style="display: block !important" src="https://cdn.rawgit.com/exokitxr/webmr-docs/media-upload/website/static/media/exokitmediacopy/exokitisnt.gif" width=300, height=auto alt="Cross out other browsers to make it clear this is not a fork"/>

## Exokit is an Exobrowser.

**Exobrowser** _n._
-	A web browser turned inside-out with its engine written in Javascript, so the browser is a site.
- This site in turn is the browser; it has full access to bindings.

<img style="display: block !important" src="https://cdn.rawgit.com/exokitxr/webmr-docs/media-upload/website/static/media/exokitmediacopy/chrome%20breaking.gif" alt="Picture of a site flipped inside out, maybe an animal, so the native insides are flipped to the outside"/>

## 2D Support

Exokit throws away support for legacy web technology that is not used in mixed reality. This is due to the fact that Exokit is a post-screen Exobrowser engine, it also is a big reason that it runs VR/AR so well.

## Why another browser?

> Chrome and Firefox are great! Solid at 3D and mixed reality too!
>
> But the problem is they straddle the 2D and 3D worlds, and something has to give. Your render loop can't be both a synchronous 2D scene while drawing a 3D scene.
>
> We needed a browser engine that starts and ends in 3D. Not an engine that starts with 20 years of 2D thinking.

*We took Exokit back to first principles.*

We didn't start with a 2D browser in order to add MR. We what imagined a runtime for VR would look like on a web stack, and then we did that. We ignored everything else.

The result is Exokit, the fastest, smallest, and most flexible 3D browser engine. It's not magic; Exokit simply does nothing between your code and your headset.

Additionally, we don't know which way the magical winds of mixed reality will blow. That's why Exokit is uniquely written in Javascript, so we are free to add API's and hardware support in a weekend, as we did for Magic Leap.

## Why should I use it?

- You want your WebGL/WebXR to run fast.
- You want the hot new web APIs.
- You want to add your own integrations, including native, into a browser environment.
- You want a lightweight browser as a hackable node module.
- You want to combine the web with a 3D engine like Unity.

## So it's written from scratch?

Yes.

We use an npm module for HTML parsing, then resurrected an old OpenGL binding from GitHub, took WebAudio extracted out of Chrome, and so on. We glued it together and wrote a custom binding for the HTML and Javascript that powers your web site.

## What platforms are supported?

Exokit Browser is just Javascript with some C++ bindings to libraries, such as OpenGL, FFmpeg, Skia, OpenVR, and others (see [here](https://exokit.org/docs/techIntegrations.html)). It builds on Windows, OSX, Linux, and binds to Vive, Oculus, Leap Motion, Magic Leap, as well as keyboard/mouse.

## How far along is the project?

Not 1.0. We need to improve site support and testing more until we call it.

If you have a site that doesn't work, there's high chance it's a silly corner case. We encourage you to [file an issue](https://github.com/exokitxr/exokit/issues/new) so we can have a look!

## Can I use it?

Sure! It's [open source on GitHub](https://github.com/exokitxr/exokit). You can also embed it since [it's a node module](https://github.com/exokitxr/exokit/blob/master/package.json).

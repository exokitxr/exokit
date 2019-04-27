---
title: Overview
section_title: SDK
type: sdk
layout: docs
order: 0
parent_section: docs
section_order: 3
---

## SDK Bundle

The [SDK bundle](installation.md) includes Exokit engine, which lets you run apps (regular web sites) in Exokit.

> Exokit is a fast browser engine, without the browser (though you can use it to build a browser!)

## SDK components

There are two main parts to the Exokit SDK:
  - Web APIs
  - Native integrations

### Web APIs

Exokit engine runs regular [HTML](https://en.wikipedia.org/wiki/HTML) sites.

If the site uses [WebXR](https://immersive-web.github.io/webxr/), [WebGL](https://www.khronos.org/registry/webgl/specs/latest/1.0/), [WebAudio](https://www.w3.org/TR/webaudio/), and other multimedia web specifications, it should work on Exokit (if it doesn't, please [file a bug](https://github.com/exokitxr/exokit/issues/new)!)

This means existing web documentation applies to Exokit as well.

**Read more at**: [Web APIs](apis/webAPIs.md)

## Native integrations

Exokit includes additional support for interfacing with hardware like Magic Leap and Leap Motion.

These are generally exposed on the `browser` object.


**Read more at**: [Native APIs](nativeAPIs.md)

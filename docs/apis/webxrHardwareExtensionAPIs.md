---
title: WebXR Hardware Extensions APIs
type: api
layout: docs
order: 2
parent_section: api
---

## WebGL

WebGL (Web Graphics Library) is a JavaScript API for rendering interactive 3D and 2D graphics within any compatible web browser without the use of plug-ins. WebGL does so by introducing an API that closely conforms to OpenGL ES 2.0 that can be used in HTML5 <canvas\> elements.

## Canvas2D

Added in HTML5, the HTML <canvas\> element can be used to draw graphics via scripting in JavaScript. For example, it can be used to draw graphs, make photo compositions, create animations, or even do real-time video processing or rendering.

## WebAudio

The Web Audio API provides a powerful and versatile system for controlling audio on the Web, allowing developers to choose audio sources, add effects to audio, create audio visualizations, apply spatial effects (such as panning) and much more.

## WebRTC

WebRTC (Web Real-Time Communications) is a technology which enables Web applications and sites to capture and optionally stream audio and/or video media, as well as to exchange arbitrary data between browsers without requiring an intermediary.

 The set of standards that comprises WebRTC makes it possible to share data and perform teleconferencing peer-to-peer, without requiring that the user install plug-ins or any other third-party software.


## Node

Node is the foundation of all that runs our JS, kind of our sturdiest foundation for the whole structure.


## OpenVR

The OpenVR API provides a game with a way to interact with Virtual Reality displays without relying on a specific hardware vendor's SDK. It can be updated independently of the game to add support for new hardware or software updates.

The API is implemented as a set of C++ interface classes full of pure virtual functions. When an application initializes the system it will return the interface that matches the header in the SDK used by that application.

 Once a version of an interface is published, it will be supported in all future versions, so the application will not need to update to a new SDK to move forward to new hardware and other features.

## Other Integrations

- OVR from Oculus		
- MLSDK from Magic Leap
- gVR from Google 
- Leap Motion
- Chromium

## More Info

To see a list of our current integrations, check out the [API overview](../overview)

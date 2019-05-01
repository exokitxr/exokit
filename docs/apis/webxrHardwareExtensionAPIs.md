---
title: WebXR Hardware Extension APIs
type: api
layout: docs
order: 2
parent_section: api
---

## OpenVR

The OpenVR API provides a game with a way to interact with Virtual Reality displays without relying on a specific hardware vendor's SDK. It can be updated independently of the game to add support for new hardware or software updates.

The API is implemented as a set of C++ interface classes full of pure virtual functions. When an application initializes the system it will return the interface that matches the header in the SDK used by that application.

 Once a version of an interface is published, it will be supported in all future versions, so the application will not need to update to a new SDK to move forward to new hardware and other features.

## Oculus VR

## Oculus Mobile

## Magic Leap

## Google VR

## Leap Motion

## Zed

## More Info

For more information, check out the [API overview](../overview).

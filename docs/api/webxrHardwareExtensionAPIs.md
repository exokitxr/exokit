---
title: WebXR Hardware Extension APIs
type: api
layout: docs
order: 2
parent_section: api
---

The WebXR Hardware Extension APIs is a generic API that is not tied to any one hardware platform and provides access to: meshing, planes detection, hand tracking, eye tracking. Although it is not a web standard yet, these are finding their way into the WebXR spec and Exokit wants to be standards-compliant.

No hardware is needed to test these APIs as Exokit Studio provides emulation and emulated data.

# Basic usage

Request a `session` instance with the `display.requestSession()` method in order to interact with XR device's presentation or tracking capabilities. Specify under `extensions` which APIs you want to access:

```js
const session = await display.requestSession({
  exclusive: true,
  extensions: {
    meshing: true,
    planesTracking: true,
    handTracking: true,
    eyeTracking: true,
  },
});
  session.addEventListener('meshadd', _meshadd);
  session.addEventListener('meshupdate', _meshupdate);
  session.addEventListener('meshremove', _meshremove);
  session.addEventListener('planeadd', _planeadd);
  session.addEventListener('planeremove', _planeremove);
```

Each extension behaves differently:
- When the meshing mechanism for an `InputSource` is invoked, update events for `meshadd`, `meshupdate`, and `meshremove` are fired on `session` (the `XRSession`).
- Planes behave similarly as meshing as there are two update events emitted: `planeadd' and 'planeremove' which are fired on `session` (the `XRSession`).
- Eye tracking exposes the eye (single eye with axes array for blink) as additional `InputSources`.
- There are two `InputSources` for hands that expose wrists which have fingers where each finger can have many bones.

Calling `getInputSources()` on `display.session` will return a list of all `InputSources` that are currently active.

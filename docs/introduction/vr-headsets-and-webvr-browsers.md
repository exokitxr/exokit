---
title: "VR Headsets & WebVR Browsers"
type: introduction
layout: docs
parent_section: introduction
order: 3
---

[w3c]: https://w3c.github.io/webvr/

<!--toc-->

## What is Virtual Reality?

Virtual reality (VR) is a technology that uses head-mounted headsets with
displays to generate the realistic images, sounds, and other sensations to put
users into an immersive virtual environment. VR allows us to create unbounded
worlds that people can walk around and interact with using their hands, to feel
as if they were transported to another place.

### What Are the Differences Between Headsets?

There are several consumer VR headsets with different features on the market.
Important distinguishing features include whether they:

- Have positional tracking (six degrees of freedom (6DoF)) or
  Only have rotational tracking (three degrees of freedom (3DoF)).
- Have controllers or not, and whether those controllers have 6DoF
  or 3DoF. Generally, the number of degrees of freedom of the controllers
  matches that of the headset.
- Are powered by a PC or by a mobile device or standalone.

Rotational tracking allows people to look around or rotate objects. All
headsets provide rotational tracking.

Positional tracking allows people to move around, get closer to objects, reach
forward. As the VR industry evolves, the minimum viable experience will trend
towards having positionally-tracked headsets with positionally-tracked
controllers. Positional tracking is important to give people presence, to make
them feel they are in a real environment. With rotational-only tracking, people
are constrained to looking around and wiggling the controller.

### What Are Some Current Headsets?

[HTC Vive]: https://www.vive.com/
[Oculus Rift]: https://www.oculus.com/rift/
[Google Daydream]: https://vr.google.com/daydream/
[Samsung GearVR]: http://www.samsung.com/global/galaxy/gear-vr/
[Windows Mixed Reality]: https://developer.microsoft.com/en-us/windows/mixed-reality/
[Oculus Go]: https://www.oculus.com/go 
[Vive Focus]: https://enterprise.vive.com/us/vivefocus/ 
[Oculus Quest]: https://www.oculus.com/quest 

| Headset                 | Platform   | Positional Tracking | Controllers        | Controller Positional Tracking |
|-------------------------|------------|---------------------|--------------------|--------------------------------|
| [HTC Vive]              | PC         | :white_check_mark:  | :white_check_mark: | :white_check_mark:             |
| [Oculus Rift]           | PC         | :white_check_mark:  | :white_check_mark: | :white_check_mark:             |
| [Google Daydream]       | Android    | :x:                 | :white_check_mark: | :x:                            |
| [Samsung GearVR]        | Android    | :x:                 | :white_check_mark: | :x:                            |
| [Windows Mixed Reality] | PC         | :white_check_mark:  | :white_check_mark: | :white_check_mark:             |
| [Oculus Go]             | Standalone | :x:                 | :white_check_mark: | :x:                            |
| [Vive Focus]            | Standalone | :x:                 | :white_check_mark: | :x:                            |
| [Oculus Quest]            | Standalone | :white_check_mark:  | :white_check_mark: | :white_check_mark:             |

## What is WebVR?

WebVR is a JavaScript API for creating immersive 3D, virtual reality
experiences in your browser. Or simply put, allows VR in the browser over the
Web.

A-Frame uses the WebVR API to gain access to VR headset sensor data (position,
orientation) to transform the camera and to render content directly to VR
headsets. Note that WebVR, which provides data, should not be confused nor
conflated with WebGL, which provides graphics and rendering.

## What Browsers Support VR?

Including [Supermedium](https://supermedium.com) and
[Exokit](https://github.com/exokitxr/exokit):

<iframe src="https://caniuse.com/#search=webxr" height="480px" width="100%"></iframe>

## Where Does A-Frame Want to Take WebVR?

This guide is from the [official A-Frame docs](https://aframe.io/docs/master/introduction/vr-headsets-and-webvr-browsers.html).

A-Frame aims for highly immersive and interactive VR content with native-like
performance. For this, A-Frame believes the minimum viable bar will trend
towards positionally-tracking headsets with positionally-tracked controllers.
This is the paradigm in which A-Frame wants to innovate as well as discover new
grounds that are specific to the VR Web (e.g., link traversal,
decentralization, identity). Contrast this type of content against flat and
static 360&deg; content and menus.

At the same time, A-Frame wants everyone to be able to get involved with VR
content creation. A-Frame supports all major headsets with their controllers.
Fortunately with the large community and contributors, A-Frame is able to both
look far towards the future as well as satisfy the needs of today's VR
landscape.

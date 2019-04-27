---
title: Using the Exokit Engine
type: introduction
layout: docs
parent_section: introduction
order: 2
source_code:
examples: []
---


## Where do I get Exokit Engine?

Install Exokit with [Exokit installer](installation.md).

> Exokit Engine will have been included with the Exokit installer.

## Open an XR site

To open a site in Exokit Engine, enter the `http`/`https` URL into the CLI. Here's an example:

```
https://threejs.org/examples/webgl_geometry_minecraft_ao.html
```

Paste this in the command window and hit _enter_. Exokit should load the site.

> Exokit expects the URL to point to a regular HTML web site which serves as the entrypoint for your app.
>
> If Exokit sees that the site requested a `WebGLRenderingContext` for a `<canvas>`, it will open a window to render the contents.

 <img src="https://cdn.rawgit.com/exokitxr/webmr-docs/media-upload/website/static/media/exokitmediacopy/runwebglsite.jpg" width=700, height=auto alt="Opening a site by entering the URL in the CLI"/>

#### Notes

- You can use a `file:///local/path/goes/here` or `file://C:\local\path\goes\here` URL to load a local file.
- This is the same thing as doing this in the Exokit CLI:
```
window.location.url = URL_GOES_HERE
```
- You can also run the Exokit Engine from any shell, assuming `exokit` is in your `PATH`:
```
exokit URL_GOES_HERE
```

## Open a WebXR site

Exokit supports both [WebXR](https://immersive-web.github.io/webxr/) and [WebVR](https://immersive-web.github.io/webvr/spec/1.1/).

> If the site is designed correctly and you have a headset, Exokit should automatically open in XR mode.

#### How it works

If you have a headset connected, Exokit will automatically emit `vrdisplayactivate` events. The site is expected to listen for these events, and start a `WebXR` or `WebVR` session in response.

The site may also listen for regular keyboard/mouse events to create an XR session, but this is not recommended as it results in a poor user experience.

## Create an engine example

There's a full [WebXR example](https://github.com/exokitxr/exokit/blob/master/examples/hello_xr.html) for you to play with on GitHub.


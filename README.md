# Exokit Browser: Javascript web browser for AR, VR, and Magic Leap

[![Slack](https://exoslack.now.sh/badge.svg)](https://exoslack.now.sh)
[![Github releases](https://img.shields.io/github/downloads/webmixedreality/exokit/total.svg)](https://github.com/webmixedreality/exokit/releases )
[![npm package](https://img.shields.io/npm/v/exokit.svg)](https://www.npmjs.com/package/exokit)
[![Travis CI build status](https://travis-ci.org/modulesio/exokit-windows.svg?branch=master)](https://travis-ci.org/modulesio/exokit-windows)
[![Appveyor build status](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?svg=true)](https://ci.appveyor.com/project/modulesio/exokit-windows)
[![Twitter Follow](https://img.shields.io/twitter/follow/webmixedreality.svg?style=social)](https://twitter.com/webmixedreality)

# Quickstart

get.webmr.io

# Introduction

[Exokit Browser](https://exokit.webmr.io) is a brand new post-screen web browser written in JavaScript as a Node module. It supports WebGL, WebVR, Magic Leap and other mixed reality hardware, and also supports vintage keyboard and mouse. And it's faster and lower latency than Chrome and Firefox in many cases, since it does less (Exokit doesn't render HTML).

The core is Javascript, so changing the browser is just like changing a site. The rest is native OpenGL and the usual libraries.

Exokit runs on Windows, macOS, and Linux.

## About Exokit Browser

Exokit can't render HTML, but it _can_ draw Canvas and WebGL -- natively, and fast -- as well as take keyboard/mouse/mixed reality input with the regular APIs. It's a browser for the post-(2D) world.

Think JSDOM, except it _actually runs_ the DOM in a `window`. Or think Electron, except 300k and no compile step. Or, think an emulator for running web sites.

The multimedia parts (e.g. WebGL) are pluggable native modules. Everything else is Javascript. It's pretty easy to experiment and add new Web APIs.

Exokit runs on Android/iOS, as well as Windows, Linux, and macOS.

## Examples

What Exokit *can* do:

- Load any `https:` site
- Parse a programmatic DOM
- Run any `<script>`
- Load `<image>`, `<video>`, `<audio>`
- Web Workers
- Canvas 2D
- WebGL
- WebVR
- Gamepad input
- Iframe isolation
- Embed anywhere with `node`
- Run on Android/iOS
- Run tests
- Power a web bot

What Exokit *cannot* do:

- Render a web page
- CSS
- Interactive HTML forms
- Legacy APIs

## FAQ

#### Why?

The web is important. The most important part of the web is that it's open. The web is not open if you need to be a genius to build a web browser.

Despite modern browsers being nominally open source, their code is impenetrable. You've probably never compiled a web browser, and almost certainly never added things. Despite the amount of time you spend in a browser.

With Exokit, anyone can write some Javascript to control their experience of the web.

#### Platform support?

Works:

- Android

Planned:

- Windows
- macOS
- iOS
- Linux

The core is Javascript and is platform-agnostic. Porting work is restricted to the native graphics APIs.

#### Web API support?

- HTTP(S)
- HTML5
- ES7 (whatever Node.js you use)
- DOM
- CanvasRenderingContext2D
- Image tag
- Audio tag
- Video tag
- Keyboard/Mouse events
- WebGL
- WebVR
- Gamepad API
- **No** HTML layout
- **No** HTML rendering
- **No** CSS

## Community

- [Slack](https://communityinviter.com/apps/exokit/exokit)

## Team

- [Avaer](//github.com/modulesio)
- [Shawn](//github.com/shawwn)
- [Noah](//github.com/NoahSchiffman)

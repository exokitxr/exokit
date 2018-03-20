# Exokit: Javascript web browser for the post-screen world

[![Travis CI build status](https://travis-ci.org/modulesio/exokit-windows.svg?branch=master)](https://travis-ci.org/modulesio/exokit-windows)
[![Appveyor build status](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?svg=true)](https://ci.appveyor.com/project/modulesio/exokit-windows)

```sh
exokit https://example.com # run site in WebGL/VR/AR
```

**Exokit** is a full HTML5 web browser, written as a node.js module. It lets you run a web site as a program.

Exokit can do everything a browser can do -- _except render HTML_.

It's plain OpenGL and standard libraries under the hood, so it plays nice with Windows, Linux, macOS, and even Magic Leap!

## What? Isn't that useless?

Exokit can't render HTML, but it _can_ draw Canvas and WebGL -- natively, and fast -- as well as take keyboard/mouse/mixed reality input with the regular APIs. It's a browser for the post-(2D) world.

Think JSDOM, except it _actually runs_ the DOM in a `window`. Or think Electron, except 300k and no compile step. Or, think an emulator for running web sites.

The multipmedia parts (e.g. WebGL) are pluggable native modules. Everything else is Javascript. It's pretty easy to experiment and add new Web APIs.

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
- A million legacy APIs
- Security; wrap it in something like Docker

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

#### Community?

- [Slack](https://communityinviter.com/apps/exokit/exokit)

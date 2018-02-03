# Exokit: JS web browser

Or, run a web site as a program.

**Exokit** is an HTML5 web browser, as a node.js module. Think JSDOM, except for running sites instead of parsing DOM. Think Chromium, except 300kb. Or think Electron, except edit-and-run instead of try-to-compile-and-give-up.

```
exokit https://example.com
```

The catch is that Exokit cannot render HTML, but it _can_ draw Canvas and WebGL -- native, fast -- as well as take keyboard/mouse/mixed reality input with the regular APIs.

The multipmedia parts (e.g. WebGL) are pluggable native modules. Everything else is Javascript. It's pretty easy to experiment and add new Web APIs.

A site is a program and `Exokit` runs web sites as programs.

Examples are illustrative.

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
- jQuery, if you really want

What Exokit *cannot* do:

- Render a web page
- CSS
- Interactive HTML forms
- A million legacy APIs
- Security; wrap it in something like Docker

## FAQ

#### Why?

The web is imporant. The most important part of the web is that it's open. The web is not open if you need to be a genius to build a web browser.

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

# Exokit: JS web browser

Or, run a web site as a program.

**Exokit** is an HTML5 web browser, as a node.js module. Think JSDOM, except for running sites instead of parsing DOM. Think Chromium, except 300kb. Or think Electron, except edit-and-run instead of try-to-compile-and-give-up.

The catch is that Exokit cannot render HTML, but it _can_ draw Canvas and WebGL -- native, fast -- as well as take keyboard/mouse/mixed reality input with the regular APIs. The multipmedia parts (e.g. WebGL) are pluggable native modules. Everything else is Javascript. Every part is editable.

The idea is that a site is a program. `Exokit` runs web programs.

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

Despite browsers being nominally open source, development is restricted to the interests of a small number of browser vendors because the code is impenetrable.

As the web transitions from 2D documents to WASM-powered multimedia, we can do better.

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
- Image
- Audio
- Video
- Keyboard/Mouse events
- WebGL
- WebVR
- Gamepad API
- *No* HTML layout
- *No* HTML rendering
- *No* CSS

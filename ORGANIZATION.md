# Directory Structure

- `assets` - Assets used in Exokit builds.
- `buildtools` - Build tools used in Windows Exokit builds.
- `deps` - Native dependency source and header files.
  - `exokit-bindings` - Native exokit bindings.
    - `bindings` - Exokit bindings native source and header files.
    - `browser` - Chrome Embedded Framework native source and header files.
    - `canvas` - Canvas native source and header files.
    - `canvascontext` - Canvas Context native source and header files.
    - `egl` - EGL bindings native source and header files.
    - `glfw` - GLFW bindings native source and header files.
    - `leapmotion` - Leap Motion bindings native source and header files.
    - `magicleap` - Magic Leap bindings native source and header files.
    - `util` - Exokit utility native source and header files.
    - `videocontext` - FFMPEG bindings native source and header files.
    - `webaudiocontext` - LabSound bindings native source and header files.
    - `webglcontext` - WebGL bindings native source and header files.
    - `webrtc` - WebRTC bindings native source and header files.
    - `windowsystem` - Window bindings native source and header files.
  - `openvr` - OpenVR native source and header files.
- `docs` - Complete documentation in Markdown.
- `examples` - 2D, WebVR, and WebXR Examples.
- `lib` - Custom THREE.js build with only the math parts.
- `metadata` - Target-specific app metadata.
- `res` - App resources for Magic Leap builds.
- `scripts` - Helper shell scripts.
- `src` - JavaScript source code.
- `tests` - Functional and unit tests with Mocha, Sinon, and Chai.
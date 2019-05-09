---
title: Command Line Flags
type: sdk
layout: docs
order: 4
parent_section: sdk
---


An example of how to use these flags:

`./exokit.cmd -hup`

`-hup` will launch Exokit Browser in a uncapped FPS mode with console performance logging.

|Short-Form Flag|Long-Form Flag|Description|
|-|-|-|
|`-v`|`--version`|Print version and exit|
|`-h`|`--home`|Loads realitytabs.html home (default)|
|`-t <site url>`|`--tab`|Load a URL as a reality tab|
|`-w`|`--webgl <1|2>`|Choose which version of WebGL to expose; defaults to 2|
|`-x <all|webxr|webvr>`|`--xr`|Choose which version of WebVR/XR to expose; defaults to all|
|`-p`|`--perf`|Log frame timing to console|
|`-s <WxH>`|`--size`|Set window size to W by H|
|`-f`|`--frame`|Log GL method calls and arguments to console|
|`-m`|`--minimalFrame`|Log GL method calls to console|
|`-q`|`--quit`|Quit on load; run the load phase and exit|
|`-l`|`--log`|Output log to log.txt|
|`-r <remote site file> <local file path>`|`--replace`|Replace file from site with a local file|
|`-u`|`--require`|Expose node require() on `window`|
|`-n`|`--nogl`|Do not create GL contexts|
|`-e`|`--headless`|Run in headless mode; do not create OS windows|
|`-d <downloadDirectory>`|`--download`|Download site to `downloadDirectory`
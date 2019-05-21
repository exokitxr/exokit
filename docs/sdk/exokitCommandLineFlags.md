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
|`-v`|`&#8209;&#8209;version`|Print version and exit|
|`-h`|`&#8209;&#8209;home`|Loads realitytabs.html home (default)|
|`-t <site url>`|`&#8209;&#8209;tab`|Load a URL as a reality tab|
|`-w`|`&#8209;&#8209;webgl <1&#124;2>`|Choose which version of WebGL to expose; defaults to 2|
|`-x <all&#124;webxr&#124;webvr>`|`&#8209;&#8209;xr`|Choose which version of WebVR/XR to expose; defaults to all|
|`-p`|`&#8209;&#8209;perf`|Log frame timing to console|
|`-s <WxH>`|`&#8209;&#8209;size`|Set window size to W by H|
|`-f`|`&#8209;&#8209;frame`|Log GL method calls and arguments to console|
|`-m`|`&#8209;&#8209;minimalFrame`|Log GL method calls to console|
|`-q`|`&#8209;&#8209;quit`|Quit on load; run the load phase and exit|
|`-l`|`&#8209;&#8209;log`|Output log to log.txt|
|`-r <remote site file> <local file path>`|`&#8209;&#8209;replace`|Replace file from site with a local file|
|`-u`|`&#8209;&#8209;require`|Expose node require() on `window`|
|`-n`|`&#8209;&#8209;nogl`|Do not create GL contexts|
|`-e`|`&#8209;&#8209;headless`|Run in headless mode; do not create OS windows|
|`-d <downloadDirectory>`|`&#8209;&#8209;download`|Download site to `downloadDirectory`|

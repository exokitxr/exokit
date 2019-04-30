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
|`-v`|`--version`|Version of Exokit Engine|
|`-h`|`--home`|Loads realitytabs.html home (default)|
|`-t`|`--tab`|Load a URL as a reality Tab|
|`-w`|`--webgl`|Exposes WebGL, by default Exokit exposes WebGL2|
|`-x`|`--xr`|By default loads both WebXR and WebVR, `-x webvr` loads WebVR specifically|
|`-p`|`--perf`|Performance logging to console|
|`-s`|`--size`|Set window size|
|`-f`|`--frame`|Get GL call list with arguments to console log|
|`-m`|`--minimalFrame`|Get GL call list to console log|
|`-q`|`--quit`|Quit and exit process|
|`-l`|`--log`|Output log to log.txt|
|`-r`|`--replace`|Replace with a local file|
|`-u`|`--require`|Require native modules|
|`-n`|`--headless`|Run Exokit in headless mode|
|`-d`|`--download`|Download site to `downloads` directory|

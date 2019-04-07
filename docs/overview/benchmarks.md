---
title: Benchmarks
type: overview
layout: docs
parent_section: overview
order: 1
source_code:
examples: []
---

## Exokit Engine vs Chrome vs Firefox

### Test Subject: [Supermedium's Gunters of OASIS](https://aframe.io/examples/showcase/gunters-of-oasis/)

### How was FPS captured?

- **Exokit Engine** - Using the flag `-up` we uncap the Exokit Engine FPS and log FPS to console.

- **Chrome** - Chrome's Dev Tools built in [FPS meter](https://developer.chrome.com/devtools/docs/rendering-settings).

- **Firefox** - Firefox's built in [FPS meter](https://developer.mozilla.org/en-US/docs/Tools/Performance/Frame_rate).

<div id='myChartContainer'>
    <canvas id="myChart" width="400" height="200"></canvas>
</div>

## Our Test Rig

- **OS** - Windows 10
- **GPU** - Nvidia GTX 980
- **CPU** - Intel i7 4790k @ 4.6 ghz
- **RAM** - 16GB @ 1800 mhz

## How do we achieve these gains?

Exokit Engine cuts the fat off of the old-school, deprecated browsers.

With less overhead, and the power of Javascript, we achieve 2-3 times faster speeds compared to old-school browsers.

Simply put, Exokit Engine does **less** work then Chrome and Firefox.

Exokit Engine gives VR/AR developers back their maximum hardware performance.

#### More sophisticated and thorough benchmarks are coming soon, but feel free to try for yourself!

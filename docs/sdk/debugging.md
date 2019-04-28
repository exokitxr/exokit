---
title: Debugging 
type: sdk
layout: docs
order: 5
parent_section: sdk
---

## Logging

Exokit has a logging mode enabled by the `-l` flag:

```sh
exokit -l https://aframe.io/a-painter/
```

The logs go in `$HOME/.exokit/log.txt`. For window that generally means `C:\username\.exokit\log.txt`.

## Debugging

You can use the regular Node debugging tools to debug Exokit Engine.

To start Exokit in inspector mode from the `exokit` git clone directory:

```sh
node --inspect .
```

After the above you should be able to use `chrome://inspect` in the Chrome browser to connect to the `node` running Exokit Engine. You can use this to:

1. Inspect values from the browser
1. Set deugging breakpoints
1. Capture heap snapshots
1. Capture CPU profiles of live sites

<!-- #### Example session -->


#### Wait for attach

You may also be interested in starting Exokit Engine so that it doesn't run until you get a chance to attach the inspector. For that you can use:

```sh
node --inspect-brk .
```

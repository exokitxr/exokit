---
title: Build from Source
section_title: SDK
type: sdk
layout: docs
order: 0
parent_section: docs
section_order: 3
---

## Common build errors
This section covers common errors when building the Exokit engine and their solutions.

### `Error: Cannot find module './build/Release/vm_one.node'`

There was a build failure that was ignored and `node_modules` contains the failure. Wipe `node_modules` and try to build again.

```sh
rm -rf node_modules
npm cache clean
```

### `<SkImage.h> not found`, or a missing header

This means a prebuilt dependency did not extract. Clear your `npm`/`yarn` cache, `rm -Rf node_modules` and `npm install` again.

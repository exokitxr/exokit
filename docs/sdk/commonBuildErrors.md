---
title: Common Build Errors 
type: sdk
layout: docs
order: 2
parent_section: sdk
---

This doc covers common errors when building the Exokit engine and their solutions.

### `Error: Cannot find module './build/Release/vm_one.node'`

There was a build failure that was ignored and node_modules contains the failure. Wipe node_modules and try to build again.

```sh
rm -rf node_modules
npm cache clean
```


---
title: Command Line
type: sdk
layout: docs
order: 3
parent_section: sdk
---


> Note: on Windows when not using bash, you'll need to use `\` intead of `/`, and you might need to `"quotes"` around certain characters.

## Prerequisites

1. [Install Exokit](../introduction/installation).
1. Navigate to your `exokit` install directory.
  - Windows default: `C:\exokit`
  - MacOS default: `/Applications/Exokit.app`
  - Linux default: `/usr/local/lib/exokit`

## Launch a URL with Exokit

On Windows:
```sh
./scripts/exokit.cmd http://yourFileURL.com/
```

On macOS and Linux:
```sh
./scripts/exokit.sh http://yourFileURL.com/
```


## Launch a local file with Exokit

On Windows:
```sh
./scripts/exokit.cmd /your/File/Path.html
```

On macOS and Linux:
```sh
./scripts/exokit.sh /your/File/Path.html
```

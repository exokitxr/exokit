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

The logs go in `$HOME/.exokit/log.txt`. For Windows that generally means `C:\username\.exokit\log.txt`.

## Web Debugging

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

## Native Debugging
You can use gdb and MinGW gdb for Windows to debug Exokit Engine.

### Using gdb
Run Exokit
```sh
cd exokit
gdb node
run src/index.js <site url>
```

Get a backtrace
```sh
bt
```

### Using MinGW gdb
[MinGW](http://www.mingw.org/) distributes a Windows version of gdb. You can [get the latest mingw installer here](http://sourceforge.net/projects/mingw/files/) which can in turn install gdb. After installing MinGW, run the "MinGW Installation Manager" (which for me was located in C:\MinGW\libexec\mingw-get\guimain.exe ) and then make sure that the mingw32-gdb bin package is installed.

There is also a fork for x64, mingw-w64, which can [be found here](https://mingw-w64.org/doku.php)).

### Using gdb
Run Exokit
```sh
cd exokit
# Path to gdb at the time of this for mingw-64 was /c/Program\ Files/mingw-w64/x86_64-8.1.0-win32-seh-rt_v6-rev0/mingw64/bin/gdb.exe
<path to gdb.exe> node
run src/index.js <site url>
```

Get a backtrace
```sh
bt
```

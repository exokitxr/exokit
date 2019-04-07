---
title: Future Native Integrations
type: api
layout: docs
order: 3
parent_section: api
---


As it stands, the current objectives for what we hope to integrate going forward are the following:

## Unity
Exokit engine is a Node.js module, so it's relatively straightforward to add native bindings to other engines like Unity, in either direction. If you have a use case you'd like supported here, let us know.

## SteamVR
Injesting SteamVR content, meaning SteamVR content and apps as another citizen to the reality tabs space, with the same feature set.

## Electron
 We often get asked how to use Exokit on top of an existing Electron app to power the WebGL/WebXR parts. This needs to be documented.

## Ethereum
This is another thing that we've experimented with for a while, and we're interested in the blockchain possibility. While we don't want Exokit to become a vehicle for token speculation, we think there could be a place for some sort of subscription model or notion of app ownership in alliance with Cryptocurrency use. This isn't strictly about Ethereum but blockchains in general.

## ARCore + ARKit
We would love to have the Exokit Engine core run on these, taking in matrices from the phone and working with any WebXR/Exokit content is a big step to take and we are all about it. We've run experiments with both so far so its definitely in the cards.

ARCore is much easier than ARKit because Apple isn't very flexible here (no dynarec, no V8). But either way this would not be a new Exokit Engine, it would be the same Exokit Engine so whatever content runs today would just auto-magically run on the phone. Like tabletop Emukit, this also applies to Daydream/VR for these devices.

* And most recently, with our newest addition of Emukit (our VR-based Console Emulator), we hope to support consoles and emulators as well.

To see a list of our current integrations, check out our [Tech Stack](../overview/techIntegrations)

---
title: iframe Reality Tabs 
type: api
layout: docs
order: 1
parent_section: api
---

Exokit implements normal [`<iframe>`](https://developer.mozilla.org/en-US/docs/Web/HTML/Element/iframe) functionality with the ability to do volumetric manipulation.


Create an iframe:
`const iframe = document.createElement('iframe');`


- `iframe.src`
    - The URL or file path.
- `iframe.positon`
    - The position vector as an array.
- `iframe.scale`
    - The scale vector as an array.
- `iframe.orientation`
    - The orientation quaternion as an array. 
- `iframe.d`
    - The dimensions of the iframe. `2` gives you DOM-to-texture. `3` gives you reality tabs.

then the `iframe` needs to be put onto the session layers:
`display.session.layers.push(iframe);`


## More Info

For a more high-level look at Reality Tabs and iframes in the Exokit engine, check out our [Reality Tabs overview](../sdk/realityTabs).

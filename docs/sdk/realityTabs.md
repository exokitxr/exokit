---
title: Reality Tabs
type: sdk
layout: docs
order: 6
parent_section: sdk
---

## Overview

The earliest records of “reality tabs” are from an issue from March and a tweet from April 2018:

<a href="https://twitter.com/webmixedreality/status/991035161797058560"><img style="display: block !important" src="https://i.imgur.com/2549u5A.png" width=auto, height=300 alt="Reality Tabs tweet April 2018"/></a>
<a href="https://github.com/exokitxr/exokit/issues/8"><img style="display: block !important" src="https://i.imgur.com/qaeyEKu.png" width=auto, height=300 alt="Reality Tabs mentioned in March 2018 github issue"/></a>

In many ways, Reality Tabs is the natural progression from current web browser tabs:

<img style="display: block !important" src="https://i.imgur.com/4FS6fC8.jpg" width=auto, height=300 alt="Too many browser tabs"/>


## What are Reality Tabs

<iframe width="560" height="315" src="https://www.youtube.com/embed/cd_DEwCDF6U" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

Reality Tabs **ARE**:  
* seperate user windows   
* web pages running in parallel   
* occupying the same 3d space   
* highly performant and optimized to run in parallel   


Reality Tabs **ARE NOT**:  
* intrinsically interactive   


## Reality Tab Use Cases

Reality Tabs open a world of possibilites for creative use cases.

You can use Reality Tabs for:  
* multiple 3d XR sites running in parallel   
* seperate XR sites communicating with eachother  
* multiple XR sites running in the same 3d space   
* XR bots that are constantly running  
* much more, be creative!  


## Technical Explanation

Reality Tabs are seperate user windows. Each Reality Tab renders at the same time in their own context before being composited into the display/headset/what have you. [Previously this was done synchornously](https://github.com/exokitxr/exokit/pull/760), one window at a time, switching GL contexts to the correct one.

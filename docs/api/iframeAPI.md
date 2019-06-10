---
title: iframe Reality Layers
type: api
layout: docs
order: 5
parent_section: api
---

Exokit implements normal [`<iframe>`](https://developer.mozilla.org/en-US/docs/Web/HTML/Element/iframe) functionality with the ability to do volumetric manipulation.


## Create an iframe
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
    - The dimensions of the iframe. `2` gives you DOM-to-texture. `3` gives you reality layers.

then the `iframe` needs to be put onto the session layers:
`display.session.layers.push(iframe);`

## iframe Reality Layers example
This section walks through the Reality Layers Exokit example. The `menumesh` refers to the GUI used in the example. See the full [realitytabs.html](https://github.com/exokitxr/exokit/blob/master/examples/realitytabs.html) example on GitHub.


![Screenshot from 2019-03-05 18-45-14](https://user-images.githubusercontent.com/29695350/57206211-b0dd3300-6f89-11e9-8c45-63835f46f658.png)

### Create the iframe
```js
const iframe = document.createElement('iframe');
iframe.onconsole = (jsString, scriptUrl, startLine) => {
  console.log('parent got console', {jsString, scriptUrl, startLine});
};
iframe.onload = function() {
  const contentDocument = (() => {
    try {
      if (this.contentDocument) { // this potentially throws
        return this.contentDocument;
      } else {
        return null;
      }
    } catch(err) {
      console.warn(err.stack);
      return null;
    }
  })();
  if (contentDocument) {
    _drawOk();
    // scene.background = null;
  } else {
    _drawFail();
    _closeTab(tab);
    if (focusedTab === tab) {
      focusedTab = rig.menuMesh.urlMesh;
    }
    rig.menuMesh.urlMesh.updateText();
    _updateRigLists();
  }
};
iframe.d = d;
iframe.src = u;
/* iframe.addEventListener('destroy', () => {
  // scene.background = _makeBackground();
}); */
const tab = _addTab(iframe, position, orientation, scale, d, local, id);
};
const _addTab = (iframe, position = new THREE.Vector3(), orientation = new THREE.Quaternion(), scale, d = 3, local = true, id = tabId++) => {
if (scale === undefined) {
  scale = new THREE.Vector3(1, d === 3 ? 1 : window.innerHeight/window.innerWidth, 1);
  if (d !== 3) {
    scale.multiplyScalar(0.4);
  }
}
iframe.position = position.toArray();
iframe.orientation = orientation.toArray();
iframe.scale = scale.toArray();
document.body.appendChild(iframe);
const tab = {
  url: iframe.src,
  id,
  iframe,
};
tabs.push(tab);
layers.push(iframe);
focusedTab = tab;
rig.menuMesh.urlMesh.updateText();
rig.menuMesh.listMesh.updateList();
return tab;
};
```

### Closing a tab
```js
const _closeTab = tab => {
const {id, iframe} = tab;
if (iframe.destroy) {
  iframe.destroy();
}
document.body.removeChild(iframe);
tabs.splice(tabs.indexOf(tab), 1);
layers.splice(layers.indexOf(iframe), 1);
if (serverConnectedUrl) {
  const objectMesh = xrmp.getObjectMeshes().find(objectMesh => objectMesh.object.id === id);
  xrmp.removeObjectMesh(objectMesh);
}
};
```

### Close all tabs
```js
const _closeAllTabs = () => { // XXX trigger this when switching servers
for (let i = 0; i < tabs.length; i++) {
  const {iframe} = tab;
  if (iframe.destroy) {
    iframe.destroy();
  }
  document.body.removeChild(iframe);
}
tabs.length = 0;
layers.length = 0;
};
```

### Send keys to iframe

```js
window.addEventListener('keydown', e => {
  if (window.document.pointerLockElement) {
    switch (e.which) {
      case 87: { // W
        keys.up = true;
        /* if (!window.document.pointerLockElement) {
          renderer.domElement.requestPointerLock();
        } */
        break;
      }
      case 83: { // S
        keys.down = true;
        /* if (!window.document.pointerLockElement) {
          renderer.domElement.requestPointerLock();
        } */
        break;
      }
      case 65: { // A
        keys.left = true;
        /* if (!window.document.pointerLockElement) {
          renderer.domElement.requestPointerLock();
        } */
        break;
      }
      case 68: { // D
        keys.right = true;
        /* if (!window.document.pointerLockElement) {
          renderer.domElement.requestPointerLock();
        } */
        break;
      }
      case 69: { // E
        fakeDisplay.gamepads[1].buttons[2].pressed = true;
        break;
      }
      case 32: { // space
        keys.space = true;
        break;
      }
      case 17: { // ctrl
        keys.ctrl = true;
        break;
      }
    }
  } else {
    if (focusedTab) {
      if (focusedTab === rig.menuMesh.urlMesh) {
        rig.menuMesh.urlMesh.handleKey(e.keyCode, e.shiftKey);
      } else if (focusedTab.iframe) {
        focusedTab.iframe.sendKeyDown(e.which, {
          shiftKey: e.shiftKey,
          ctrlKey: e.ctrlKey,
          altKey: e.altKey,
        });
      }
    }
  }
});
window.addEventListener('keyup', e => {
  if (window.document.pointerLockElement) {
    switch (e.which) {
      case 87: { // W
        keys.up = false;
        break;
      }
      case 83: { // S
        keys.down = false;
        break;
      }
      case 65: { // A
        keys.left = false;
        break;
      }
      case 68: { // D
        keys.right = false;
        break;
      }
      case 69: { // E
        fakeDisplay.gamepads[1].buttons[2].pressed = false;
        break;
      }
      case 32: { // space
        keys.space = false;
        break;
      }
      case 17: { // ctrl
        keys.ctrl = false;
        break;
      }
    }
  } else {
    if (focusedTab && focusedTab.iframe) {
      focusedTab.iframe.sendKeyUp(e.which, {
        shiftKey: e.shiftKey,
        ctrlKey: e.ctrlKey,
        altKey: e.altKey,
      });
    }
  }
});
window.addEventListener('keypress', e => {
  if (!window.document.pointerLockElement) {
    if (focusedTab) {
      /* if (focusedTab === rig.menuMesh.urlMesh) {
        rig.menuMesh.urlMesh.handleKey(e.keyCode, e.shiftKey);
      } else */if (focusedTab.iframe) {
        focusedTab.iframe.sendKeyPress(e.which, {
          shiftKey: e.shiftKey,
          ctrlKey: e.ctrlKey,
          altKey: e.altKey,
        });
      }
    }
  }
});
```

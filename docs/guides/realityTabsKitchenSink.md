---
title: Kitchen Sink Example
type: guides
layout: docs
order: 3
parent_section: Reality Tabs
---

<iframe width="560" height="315" src="https://www.youtube.com/embed/RUDqv6uO7Ac" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

The "Kitchen Sink" reality tabs example is a showcase of a few of the Magic Leap APIs: meshing, eye-tracking, plane-tracking, hand-tracking, just about everything but the kitchen sink!

## Magic Leap API

The following are how the example utilizes Exokit's [Magic Leap API](../MagicLeapAPI).

- Enabling
```js
      let enabled = false;
      const _enable = () => {
        mesher = window.browser.magicleap.RequestMeshing();
        mesher.onmesh = _onMesh;
        planeTracker = window.browser.magicleap.RequestPlaneTracking();
        planeTracker.onplanes = _onPlanes;
        handTracker = window.browser.magicleap.RequestHandTracking();
        handTracker.onhands = _onHands;
        handMesh.visible = true;
        eyeTracker = window.browser.magicleap.RequestEyeTracking();
        enabled = true;
      };
```

- Meshing / _onMesh
```js
      const _onMesh = updates => {
        for (let i = 0; i < updates.length; i++) {
          const update = updates[i];
          const {id, type} = update;
          if (type === 'new' || type === 'update') {
            _loadTerrainMesh(_getTerrainMesh(id), update);
          } else if (type === 'unchanged') {
            // nothing
          } else {
            const index = terrainMeshes.findIndex(terrainMesh => terrainMesh.meshId === id);
            if (index !== -1) {
              const terrainMesh = terrainMeshes[index];
              _removeTerrainMesh(terrainMesh);
              terrainMeshes.splice(index, 1);
            }
          }
        }
      };
```

- Eye tracking / eyeTracker
```js
      if (eyeTracker) {
        const {fixation} = eyeTracker;
        const {position, rotation} = fixation;
        eyeMesh.position.fromArray(position);
        eyeMesh.quaternion.fromArray(rotation);
        eyeMesh.updateMatrix();
        eyeMesh.updateMatrixWorld();
        eyeMesh.visible = true;
      } else {
        eyeMesh.visible = false;
      }
```

- Plane-tracking / planeMeshes / _onPlanes
```js
      const planeMeshes = [];
      const planeGeometry = new THREE.PlaneBufferGeometry(1, 1);
      const planeMaterial = new THREE.MeshPhongMaterial({
        color: 0xFF0000,
      });
```
```js
      const _loadPlanes = planes => {
        _clearPlanes();
        for (let i = 0; i < planes.length; i++) {
          const plane = planes[i];
          const mesh = new THREE.Mesh(planeGeometry, planeMaterial);
          mesh.frustumCulled = false;
          mesh.position.fromArray(plane.position);
          mesh.quaternion.fromArray(plane.rotation);
          mesh.scale.set(plane.size[0], plane.size[1], 1);
          mesh.updateMatrix();
          mesh.updateMatrixWorld();
          scene.add(mesh);
          planeMeshes.push(mesh);
        }
      };
```
```js
      const _clearPlanes = () => {
        for (let i = 0; i < planeMeshes.length; i++) {
          scene.remove(planeMeshes[i]);
        }
        planeMeshes.length = 0;
      };
```
```js
      const _onPlanes = planes => {
        _loadPlanes(planes);
      };
```

- Hand-tracking / handMesh /  _onHands
```js
        const mesh = new THREE.Mesh(geometry, material);
        mesh.update = hands => {
          let positionIndex = 0;
          let indexIndex = 0;
          const _shiftIndex = array => {
            for (let l = 0; l < array.length; l++) {
              array[l] += positionIndex / 3;
            }
          };
          for (let i = 0; i < hands.length; i++) {
            const hand = hands[i];
            const {pointer, grip, wrist, fingers} = hand;
            const allFingers = [wrist].concat(fingers);
            for (let j = 0; j < allFingers.length; j++) {
              const bones = allFingers[j];
              for (let k = 0; k < bones.length; k++) {
                const positionFloat32Array = bones[k];
                if (positionFloat32Array) {
                  const position = localVector.fromArray(positionFloat32Array);
                  const newGeometry = boneGeometry.clone()
                    .applyMatrix(
                      localMatrix.makeTranslation(position.x, position.y, position.z)
                    );
                  _shiftIndex(newGeometry.index.array);
                  positions.set(newGeometry.attributes.position.array, positionIndex);
                  positionIndex += newGeometry.attributes.position.array.length;
                  indices.set(newGeometry.index.array, indexIndex);
                  indexIndex += newGeometry.index.array.length;
                }
              }
            }
```
```js
            if (pointer) {
              const newGeometry = pointerGeometry.clone()
                .applyMatrix(
                  localMatrix.compose(
                    localVector.fromArray(pointer.position),
                    localQuaternion.fromArray(pointer.rotation),
                    localVector2.set(1, 1, 1),
                  )
                );
              _shiftIndex(newGeometry.index.array);
              positions.set(newGeometry.attributes.position.array, positionIndex);
              positionIndex += newGeometry.attributes.position.array.length;
              indices.set(newGeometry.index.array, indexIndex);
              indexIndex += newGeometry.index.array.length;
            }
```
```js
            if (grip) {
              const newGeometry = pointerGeometry.clone()
                .applyMatrix(
                  localMatrix.compose(
                    localVector.fromArray(grip.position),
                    localQuaternion.fromArray(grip.rotation),
                    localVector2.set(1, 1, 1),
                  )
                );
              _shiftIndex(newGeometry.index.array);
              positions.set(newGeometry.attributes.position.array, positionIndex);
              positionIndex += newGeometry.attributes.position.array.length;
              indices.set(newGeometry.index.array, indexIndex);
              indexIndex += newGeometry.index.array.length;
            }
          }
```
```js
          positionAttribute.needsUpdate = true;
          indexAttribute.needsUpdate = true;
          geometry.setDrawRange(0, indexIndex);
        };
        mesh.visible = false;
        mesh.frustumCulled = false;
        return mesh;
      })();
      scene.add(handMesh);
```
```js
      const _onHands = hands => {
        handMesh.update(hands);
      };
```

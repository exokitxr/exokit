const path = require('path');
const fs = require('fs');

const THREE = require('three-zeo');

const oneVector = new THREE.Vector3(1, 1, 1);
const localVector = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();

const nativeHtml = document.createElement('native-html');
nativeHtml.show();

let canvasWidth = 1280;
let canvasHeight = 1024;
const canvas = document.createElement('native-canvas', canvasWidth, canvasHeight);
const gl = canvas.getContext('webgl');

const _requestJsonFile = p => new Promise((accept, reject) => {
  fs.readFile(p, (err, s) => {
    if (!err) {
      accept(JSON.parse(s));
    } else {
      reject(err);
    }
  });
});
const _requestJsonMesh = (modelJson, modelTexturePath) => new Promise((accept, reject) => {
  const loader = new THREE.ObjectLoader();
  loader.setTexturePath(modelTexturePath);
  loader.parse(modelJson, accept);
});

const controllerjsPath = path.join(require.resolve('controllerjs'), '..');
Promise.all([
  _requestJsonFile(path.join(controllerjsPath, 'model', 'controller.json'))
    .then(controllerJson => _requestJsonMesh(controllerJson, path.join(controllerjsPath, 'model', '/'))),
  navigator.getVRDisplays()
    .then(displays => displays[0]),
])
  .then(([
    controllerModel,
    display,
  ]) => {
    const renderer = new THREE.WebGLRenderer({
      canvas: canvas,
      context: gl,
      antialias: true,
    });
    // renderer.setSize(canvas.width, canvas.height);
    renderer.setClearColor(0xffffff, 1);

    const scene = new THREE.Scene();

    const _makeCamera = () => {
      const camera = new THREE.PerspectiveCamera(90, canvasWidth / canvasHeight, 0.1, 1000);
      camera.position.set(0, 0, 2);
      camera.lookAt(new THREE.Vector3(0, 0, 0));
      return camera;
    };
    let camera = _makeCamera();
    scene.add(camera);

    const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
    directionalLight.position.set(1, 1, 1);
    scene.add(directionalLight);

    const boxMesh = (() => {
      const geometry = new THREE.BoxBufferGeometry(1, 1, 1);
      const material = new THREE.MeshPhongMaterial({
        color: 0xFF0000,
      });
      return new THREE.Mesh(geometry, material);
    })();
    scene.add(boxMesh);

    const leftControllerMesh = controllerModel.children[0].clone(true);
    scene.add(leftControllerMesh);

    const rightControllerMesh = controllerModel.children[0].clone(true);
    scene.add(rightControllerMesh);

    const _render = () => {
      let gamepads = navigator.getGamepads() || [];
      
      const leftGamepad = gamepads.find(gamepad => gamepad && gamepad.hand === 'left');
      if (leftGamepad) {
        leftControllerMesh.matrix.compose(localVector.fromArray(leftGamepad.position), localQuaternion.fromArray(leftGamepad.quaternion), oneVector);
        leftControllerMesh.matrix.decompose(leftControllerMesh.position, leftControllerMesh.quaternion, leftControllerMesh.scale);
        leftControllerMesh.updateMatrixWorld();
      }

      const rightGamepad = gamepads.find(gamepad => gamepad && gamepad.hand === 'right');
      if (rightGamepad) {
        rightControllerMesh.matrix.compose(localVector.fromArray(rightGamepad.position), localQuaternion.fromArray(rightGamepad.quaternion), oneVector);
        rightControllerMesh.matrix.decompose(rightControllerMesh.position, rightControllerMesh.quaternion, rightControllerMesh.scale);
        rightControllerMesh.updateMatrixWorld();
      }

      renderer.render(scene, camera);
      renderer.context.flush();

      requestAnimationFrame(_render);
    };
    requestAnimationFrame(_render);

    canvas.on('resize', e => {
      console.log('resize', e);

      canvasWidth = e.width;
      canvasHeight = e.height;
      
      canvas.style.width = canvasWidth;
      canvas.style.height = canvasHeight;

      if (display && !display.isPresenting) {
        renderer.setSize(canvasWidth, canvasHeight);
        camera.aspect = canvasWidth / canvasHeight;
        camera.updateProjectionMatrix();
      }
    });
    canvas.on('mousemove', e => {
      if (canvas.pointerLockElement) {
        e.deltaX = e.pageX - (canvasWidth / 2);
        e.deltaY = e.pageY - (canvasHeight / 2);

        canvas.setCursorPos(canvasWidth / 2, canvasHeight / 2);
      } else  {
        e.deltaX = 0;
        e.deltaY = 0;
      }

      console.log('mousemove', e);
    });
    canvas.on('mousedown', e => {
      console.log('mousedown', e);

      if ((e.pageX / canvasWidth) < 0.5) {
        if (!canvas.pointerLockElement) {
          canvas.requestPointerLock();
        }
      } else {
        if (display && !display.isPresenting) {
          return display.requestPresent([
            {
              leftBounds: [0, 0, 0.5, 1],
              rightBounds: [0.5, 0, 0.5, 1],
              source: canvas,
            },
          ])
          .then(() => {
            renderer.vr.enabled = true;
            // renderer.vr.standing = true;
            renderer.vr.setDevice(display);

            const leftEye = display.getEyeParameters('left');
            const rightEye = display.getEyeParameters('right');
            const width = Math.max(leftEye.renderWidth, rightEye.renderWidth) * 2;
            const height = Math.max(leftEye.renderHeight, rightEye.renderHeight);
            renderer.setSize(width, height);
            
            canvas.style.width = canvasWidth;
            canvas.style.height = canvasHeight;
          })
          .catch(err => {
            console.warn(err);
          });
        }
      }
    });
    canvas.on('mouseup', e => {
      console.log('mouseup', e);
    });
    canvas.on('keydown', e => {
      console.log('keyup', e);
      if (e.keyCode === 27) { // esc
        if (canvas.pointerLockElement) {
          canvas.exitPointerLock();
        } else if (display && display.isPresenting) {
          renderer.vr.enabled = false;
          renderer.vr.setDevice(null);

          renderer.setSize(canvasWidth, canvasHeight);

          scene.remove(camera);
          camera = _makeCamera();
          scene.add(camera);

          display.exitPresent()
            .then(() => {
              // nothing
            })
            .catch(err => {
              console.warn(err);
            });
        }
      }
    });
    canvas.on('keyup', e => {
      console.log('keyup', e);
    });
    canvas.on('keypress', e => {
      console.log('keypress', e);
    });
    canvas.on('quit', () => {
      process.exit(0);
    });
  })
  .catch(err => {
    console.warn(err.stack);
    process.exit(1);
  });

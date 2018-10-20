(() => {

const localVector = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localVector2 = new THREE.Vector3();
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();
const localViewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);

window._makeFakeDisplay = () => {
  const fakeDisplay = window.navigator.createVRDisplay();
  fakeDisplay.setSize(window.innerWidth * window.devicePixelRatio * 2, window.innerHeight * window.devicePixelRatio);
  fakeDisplay.getEyeParameters = (getEyeParameters => function(eye) {
    if (!fakeDisplay.getStereo() && eye === 'right') {
      const result = getEyeParameters.call(this, 'right');
      result.renderWidth = 0;
      return result;
    } else {
      return getEyeParameters.apply(this, arguments);
    }
  })(fakeDisplay.getEyeParameters);
  fakeDisplay.onrequestanimationframe = fn => window.requestAnimationFrame(fn);
  fakeDisplay.onvrdisplaypresentchange = () => {
    setTimeout(() => {
      const e = new Event('vrdisplaypresentchange');
      e.display = fakeDisplay;
      window.dispatchEvent(e);
    });
  };
  fakeDisplay.requestSession = function() {
    return Promise.resolve(this.session);
  };
  fakeDisplay.enter = ({canvas, animate, stereo = false}) => {
    if (fakeDisplay.session) {
      fakeDisplay.session.end();
    }

    fakeDisplay.requestPresent([{source: canvas}])
      .then(() => {
        renderer.vr.enabled = true;
        renderer.vr.setDevice(fakeDisplay);
        renderer.vr.setAnimationLoop(animate);

        if (navigator.xr) {
          const onends = [];
          const session = {
            addEventListener(e, fn) {
              if (e === 'end') {
                onends.push(fn);
              }
            },
            device: fakeDisplay,
            baseLayer: null,
            _frame: null, // defer
            getInputSources() {
              return [];
            },
            requestFrameOfReference() {
              return Promise.resolve({});
            },
            requestAnimationFrame(fn) {
              return this.device.onrequestanimationframe(timestamp => {
                fn(timestamp, this._frame);
              });
            },
            end() {
              for (let i = 0; i < onends.length; i++) {
                onends[i]();
              }
              return fakeDisplay.exitPresent();
            },
            clone() {
              const o = new this.constructor();
              for (const k in this) {
                o[k] = this[k];
              }
              o._frame = o._frame.clone();
              o._frame.session = o;
              return o;
            },
          };
          const _frame = {
            session,
            views: !stereo ? [{
              eye: 'left',
              projectionMatrix: fakeDisplay._frameData.leftProjectionMatrix,
              _viewport: {
                x: 0,
                y: 0,
                width: fakeDisplay._width,
                height: fakeDisplay._height,
              },
            }] : [
              {
                eye: 'left',
                projectionMatrix: fakeDisplay._frameData.leftProjectionMatrix,
                _viewport: {
                  x: 0,
                  y: 0,
                  width: fakeDisplay._width/2,
                  height: fakeDisplay._height,
                },
              },
              {
                eye: 'right',
                projectionMatrix: fakeDisplay._frameData.rightProjectionMatrix,
                _viewport: {
                  x: fakeDisplay._width,
                  y: 0,
                  width: fakeDisplay._width/2,
                  height: fakeDisplay._height,
                },
              },
            ],
            _pose: null, // defer
            getDevicePose() {
              return this._pose;
            },
            clone() {
              const o = new this.constructor();
              for (const k in this) {
                o[k] = this[k];
              }
              o._pose = o._pose.clone();
              o._pose.frame = o;
              return o;
            },
          };
          session._frame = _frame;
          const _pose = {
            frame: _frame,
            getViewMatrix(view) {
              const viewMatrix = this.frame.session.device._frameData[view.eye === 'left' ? 'leftViewMatrix' : 'rightViewMatrix'];

              if (this.frame && this.frame.session && this.frame.session.device && this.frame.session.device.window) {
                const {xrOffset} = this.frame.session.device.window.document;
                localMatrix
                  .fromArray(viewMatrix)
                  .multiply(
                    localMatrix2.compose(
                      localVector.fromArray(xrOffset.position),
                      localQuaternion.fromArray(xrOffset.rotation),
                      localVector2.fromArray(xrOffset.scale)
                    )
                  )
                  .toArray(localViewMatrix);
              } else {
                localViewMatrix.set(viewMatrix);
              }
              return localViewMatrix;
            },
            clone() {
              const o = new this.constructor();
              for (const k in this) {
                o[k] = this[k];
              }
              return o;
            },
          };
          _frame._pose = _pose;
          fakeDisplay.session = session;
          renderer.vr.setSession(session, {
            frameOfReferenceType: 'stage',
          });
        } else {
          window.dispatchEvent(new Event('vrdisplaypresentchange'));
        }

        fakeDisplay.stereo = stereo;
      });
  };

  return fakeDisplay;
};

})();

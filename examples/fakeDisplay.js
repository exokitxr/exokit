const _makeFakeDisplay = () => {
  const fakeDisplay = window.navigator.createVRDisplay();
  fakeDisplay.setSize(window.innerWidth * window.devicePixelRatio, window.innerHeight * window.devicePixelRatio);
  fakeDisplay.getEyeParameters = (getEyeParameters => function(eye) {
    if (!fakeDisplay.getStereo() && eye === 'right') {
      const result = getEyeParameters.call(this, 'right');
      result.renderWidth = 0;
      return result;
    } else {
      return getEyeParameters.apply(this, arguments);
    }
  })(fakeDisplay.getEyeParameters);
  fakeDisplay.stereo = false;
  fakeDisplay.update();
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
        renderer.vr.setDevice(fakeDisplay);
        renderer.vr.setAnimationLoop(animate);

        if (navigator.xr) {
          const views = !stereo ? [{
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
                width: fakeDisplay._width / 2,
                height: fakeDisplay._height,
              },
            },
            {
              eye: 'right',
              projectionMatrix: fakeDisplay._frameData.rightProjectionMatrix,
              _viewport: {
                x: fakeDisplay._width / 2,
                y: 0,
                width: fakeDisplay._width / 2,
                height: fakeDisplay._height,
              }
            },
          ];
          const devicePose = {
            getViewMatrix(view) {
              return fakeDisplay._frameData[view.eye === 'left' ? 'leftViewMatrix' : 'rightViewMatrix'];
            },
          };
          const frame = {
            views,
            getDevicePose() {
              return devicePose;
            },
          };
          const onends = [];
          const session = {
            addEventListener(e, fn) {
              if (e === 'end') {
                onends.push(fn);
              }
            },
            baseLayer: null,
            getInputSources() {
              return [];
            },
            requestFrameOfReference() {
              return Promise.resolve(stereo ? {} : null);
            },
            device: fakeDisplay,
            requestAnimationFrame(fn) {
              return this.device.onrequestanimationframe(timestamp => {
                fn(timestamp, frame);
              });
            },
            /* requestAnimationFrame: fn => {
              const handler = timestamp => {
                fn(timestamp, frame);
              };
              if (window.tickAnimationFrame && window.tickAnimationFrame.window) {
                return window.tickAnimationFrame.window.requestAnimationFrame(handler);
              } else {
                return window.requestAnimationFrame(handler);
              }
            }, */
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
              return o;
            },
          };
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

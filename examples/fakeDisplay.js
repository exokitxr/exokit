(() => {

window._makeFakeDisplay = () => {
  const fakeDisplay = window.navigator.createVRDisplay();
  fakeDisplay.enter = async ({renderer, animate, layers}) => {
    const {domElement: canvas} = renderer;

    if (navigator.xr) {
      const session = await fakeDisplay.requestSession({
        exclusive: true,
      });

      await new Promise((accept, reject) => {
        session.requestAnimationFrame((timestamp, frame) => {
          fakeDisplay.layers = layers;

          renderer.vr.enabled = true;
          renderer.vr.setDevice(fakeDisplay);
          renderer.vr.setSession(session, {
            frameOfReferenceType: 'stage',
          });
          renderer.vr.setAnimationLoop(animate);
          
          const viewport = session.baseLayer.getViewport(frame.views[0]);
          const height = viewport.height;
          const fullWidth = (() => {
            let result = 0;
            for (let i = 0; i < frame.views.length; i++) {
              result += session.baseLayer.getViewport(frame.views[i]).width;
            }
            return result;
          })();
          renderer.setSize(fullWidth, height);
          
          accept();
        });
      });
    } else {
      await fakeDisplay.requestPresent([
        {
          source: canvas,
        },
      ]);
      
      fakeDisplay.layers = layers;

      renderer.vr.enabled = true;
      renderer.vr.setDevice(fakeDisplay);
      renderer.vr.setAnimationLoop(animate);
 
      // const {renderWidth: width, renderHeight: height} = fakeDisplay.getEyeParameters('left');
      // renderer.setSize(width * 2, height);
    }
  };

  return fakeDisplay;
};

})();

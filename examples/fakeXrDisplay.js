(() => {

window._makeFakeXrDisplay = () => {
  const fakeXrDisplay = navigator.createFakeXRDisplay();
  fakeXrDisplay.enter = async ({renderer, animate, layers}) => {
    const {domElement: canvas} = renderer;

    if (navigator.xr) {
      const session = await navigator.xr.requestSession({
        exclusive: true,
      });
      
      await new Promise((accept, reject) => {
        session.requestAnimationFrame((timestamp, frame) => {
          session.layers = layers;

          renderer.vr.enabled = true;
          renderer.vr.setSession(session, {
            frameOfReferenceType: 'stage',
          });
          renderer.vr.setAnimationLoop(animate);
          
          const viewport = session.renderState.baseLayer.getViewport(frame.views[0]);
          const height = viewport.height;
          const fullWidth = (() => {
            let result = 0;
            for (let i = 0; i < frame.views.length; i++) {
              result += session.renderState.baseLayer.getViewport(frame.views[i]).width;
            }
            return result;
          })();
          renderer.setSize(fullWidth, height);
          
          accept();
        });
      });
    } else {
      const display = await navigator.getVRDisplays()[0];
      await display.requestPresent([
        {
          source: canvas,
        },
      ]);
      
      display.layers = layers;

      renderer.vr.enabled = true;
      renderer.vr.setDevice(display);
      renderer.vr.setAnimationLoop(animate);
 
      // const {renderWidth: width, renderHeight: height} = display.getEyeParameters('left');
      // renderer.setSize(width * 2, height);
    }
  };

  return fakeXrDisplay;
};

})();

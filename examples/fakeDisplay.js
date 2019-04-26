(() => {

window._makeFakeDisplay = () => {
  const fakeDisplay = window.navigator.createVRDisplay();
  fakeDisplay.enter = async ({renderer, animate, layers}) => {
    const {domElement: canvas} = renderer;

    if (navigator.xr) {
      const session = await fakeDisplay.requestSession({
        exclusive: true,
      });

      fakeDisplay.layers = layers;

      renderer.vr.enabled = true;
      renderer.vr.setDevice(fakeDisplay);
      renderer.vr.setSession(session, {
        frameOfReferenceType: 'stage',
      });
      renderer.vr.setAnimationLoop(animate);
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
    }
  };

  return fakeDisplay;
};

})();

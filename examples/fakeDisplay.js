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

    const context = renderer.getContext();
    const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = window.browser.createRenderTarget(context, canvas.width, canvas.height, 0, 0, 0, 0);
    context.setDefaultFramebuffer(msFbo);
    canvas.framebuffer = {
      msTex,
      msDepthTex,
      tex,
      depthTex,
    };
  };

  return fakeDisplay;
};

})();

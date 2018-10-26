(() => {
  const canvas = document.createElement('canvas');
  const ctx = canvas.getContext('2d');
  document.body.appendChild(canvas);

  const ws = new WebSocket((window.location.protocol === 'https:' ? 'wss' : 'ws') + '://' + window.location.host + '/');
  ws.binaryType = 'blob';
  ws.onopen = () => {
    console.log('connected');
  };

  let running = false;
  let pendingData = null;
  const _handleData = async data => {
    if (!running) {
      running = true;

      try {
        const imgBitmap = await createImageBitmap(data);

        if (canvas.width !== imgBitmap.width) {
          canvas.width = imgBitmap.width;
        }
        if (canvas.height !== imgBitmap.height) {
          canvas.height = imgBitmap.height;
        }

        ctx.drawImage(imgBitmap, 0, 0);
      } catch (err) {
        console.warn(err.stack);
      } finally {
        running = false;
        _next();
      }
    } else {
      pendingData = data;
    }
  };
  const _next = () => {
    if (pendingData) {
      const localPendingData = pendingData;
      pendingData = null;
      _handleData(localPendingData);
    }
  };
  ws.onmessage = m => {
    _handleData(m.data);
  };
})();

(() => {
  const canvas = document.createElement('canvas');
  const ctx = canvas.getContext('2d');
  document.body.appendChild(canvas);

  let id = 0;

  const ws = new WebSocket((window.location.protocol === 'https:' ? 'wss' : 'ws') + '://' + window.location.host + '/');
  ws.binaryType = 'arraybuffer';
  ws.onopen = () => {
    console.log('connected');
  };

  let running = false;
  let pendingData = null;
  const _handleData = async data => {
    if (!running) {
      running = true;

      data = JSON.parse(data);

      try {
        const img = await new Promise((accept, reject) => {
          const img = new Image();
          img.onload = () => {
            accept();
          };
          img.onerror = err => {
            reject(err);
          };
          img.src = '/frame/' + (id++);
        });
        
        if (canvas.width !== data.width) {
          canvas.width = result.data.width;
        }
        if (canvas.height !== data.height) {
          canvas.height = data.height;
        }
        
        ctx.drawImage(img, 0, 0);
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

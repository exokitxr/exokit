(() => {
  const canvas = document.createElement('canvas');
  const ctx = canvas.getContext('2d');
  document.body.appendChild(canvas);
  
  const ws = new WebSocket((window.location.protocol === 'https:' ? 'wss' : 'ws') + '://' + window.location.host + '/');
  ws.binaryType = 'arraybuffer';
  ws.onopen = () => {
    console.log('connected');
  };

  let running = false;
  let pendingData = null;
  const _handleData = data => {
    if (!running) {
      running = true;

      const datas = JSON.parse(data);
      if (datas.length > 0) {
        Promise.all(datas.map(data =>
          fetch('/frame/' + data.index)
            .then(res => res.arrayBuffer())
            .then(arrayBuffer => ({
              data,
              arrayBuffer,
            }))
        ))
          .then(results => {
            // console.log('got frame');
            /* console.log('format', {
              // Many video formats require an 8- or 16-pixel block size.
              width: results[0].data.width,
              height: results[0].data.height,

              // Using common 4:2:0 layout, chroma planes are halved in each dimension.
              chromaWidth: results[0].data.width / 2,
              chromaHeight: results[0].data.height / 2,

              texWidth: results[3].data.width,
              texHeight: results[3].data.height,

              // Crop out a 1920x1080 visible region:
              cropLeft: 0,
              cropTop: 0,
              cropWidth: results[0].data.width,
              cropHeight: results[0].data.height,

              // Square pixels, so same as the crop size.
              displayWidth: width,
              displayHeight: height,
            }, {
              y: {
                bytes: new Uint8Array(results[0].arrayBuffer),
                stride: results[0].data.stride,
              },
              u: {
                bytes: new Uint8Array(results[1].arrayBuffer),
                stride: results[1].data.stride,
              },
              v: {
                bytes: new Uint8Array(results[2].arrayBuffer),
                stride: results[2].data.stride,
              },
              c: {
                bytes: new Uint8Array(results[3].arrayBuffer),
                stride: results[3].data.stride,
              },
            }); */

            // window.results = results;
            // console.log('got data', new Uint8Array(results[0].arrayBuffer));

            if (canvas.width !== results[0].data.width) {
              canvas.width = results[0].data.width;
            }
            if (canvas.height !== results[0].data.height) {
              canvas.height = results[0].data.height;
            }
            
            const imageData = ctx.createImageData(results[0].data.width, results[0].data.height);
            imageData.data.set(new Uint8Array(results[0].arrayBuffer));
            ctx.putImageData(imageData, 0, 0);
          })
          .then(() => {
            running = false;
            _next();
          })
          .catch(err => {
            console.warn(err.stack);

            running = false;
            _next();
          });
      } else {
        running = false;
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

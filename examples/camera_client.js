(() => {
  const width = 1280;
  const height = 1024;

  const ScreenQuad = (() => {
    var defaultQuad = new THREE.PlaneBufferGeometry(2, 2, 1, 1);

    var defaultVertexShader = [

      `varying vec2 vUv;
		  void main(){
			  vUv = uv;
			  gl_Position = vec4(position.xy, 1., 1.);
		  }`,

    ].join("\n");

    var defaultFragmentShader = [

      `uniform sampler2D uTexture;
      uniform sampler2D uContent;
      varying vec2 vUv;
      void main() {
         vec4 t = texture2D(uTexture, vUv);
         vec4 c = texture2D(uContent, vUv);

         /* // Y, Cb, and Cr planes are uploaded as LUMINANCE textures.
         float fY = t.r;
         float fCb = t.g;
         float fCr = t.b;
         
         // Premultipy the Y...
         float fYmul = fY * 1.1643828125; */

         // And convert that to RGB!
         gl_FragColor = vec4(t.rgb, 1.);
         /* gl_FragColor = vec4(
           fYmul + 1.59602734375 * fCr - 0.87078515625,
           fYmul - 0.39176171875 * fCb - 0.81296875 * fCr + 0.52959375,
           fYmul + 2.017234375   * fCb - 1.081390625,
           1.
         ); */
         gl_FragColor.rgb = (gl_FragColor.rgb * (1.0 - c.a)) + (c.rgb * c.a);
      }`

    ].join('\n');


    function ScreenQuad( params ){

      params = params || {};

      THREE.Mesh.apply( this, [ defaultQuad , new THREE.ShaderMaterial({

        uniforms:{
          uTexture: {
            type: 't',
            value: undefined !== params.uTexture ? params.uTexture : null,
          },
          uContent: {
            type: 't',
            value: undefined !== params.uContent ? params.uContent : null
          },
        },

        vertexShader: defaultVertexShader,

        fragmentShader: defaultFragmentShader,

        depthWrite: false,

      })]);

      this.frustumCulled = false;

      this.renderOrder = -1;

      //end mesh setup

      // console.log( this , this.width , this.height );

    }

    ScreenQuad.prototype = Object.create( THREE.Mesh.prototype );

    ScreenQuad.constructor = ScreenQuad;

    return ScreenQuad;

  })();

  var camera, scene, renderer, mesh;

  init();
  animate();

  function init() {

    camera = new THREE.PerspectiveCamera( 50, window.innerWidth / window.innerHeight, 1, 10 );
    camera.position.z = 2;

    scene = new THREE.Scene();
    scene.background = new THREE.Color( 0x101010 );

    const uTexture = new THREE.Texture(
      null,
      THREE.UVMapping,
      THREE.ClampToEdgeWrapping,
      THREE.ClampToEdgeWrapping,
      THREE.LinearMipMapNearestFilter,
      THREE.LinearMipMapNearestFilter,
      THREE.RGBAFormat,
      THREE.UnsignedByteType,
      1
    );
    const uContent = new THREE.Texture(
      null,
      THREE.UVMapping,
      THREE.ClampToEdgeWrapping,
      THREE.ClampToEdgeWrapping,
      THREE.LinearMipMapNearestFilter,
      THREE.LinearMipMapNearestFilter,
      THREE.RGBAFormat,
      THREE.UnsignedByteType,
      1
    );
    mesh = new ScreenQuad({
      uTexture,
      uContent,
    });
    scene.add( mesh );

    renderer = new THREE.WebGLRenderer();
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    document.body.appendChild( renderer.domElement );

    window.addEventListener( 'resize', onWindowResize, false );

  }

  function onWindowResize( event ) {

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth, window.innerHeight );

  }

  //

  function animate() {

    requestAnimationFrame( animate );

    render();

  }

  function render() {

    renderer.render( scene, camera );

  }

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
      if (datas.length >= 2) {
        Promise.all(datas.map(data =>
          fetch('/frame/' + data.index)
            .then(res => res.arrayBuffer())
            .then(arrayBuffer => ({
              data,
              arrayBuffer,
            }))
        ))
          .then(results => {
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

            window.results = results;
            // console.log('got data', new Uint8Array(results[0].arrayBuffer));

            if (
              !mesh.material.uniforms.uTexture.value ||
              mesh.material.uniforms.uTexture.value.width !== results[0].data.width ||
              mesh.material.uniforms.uTexture.value.height !== results[0].data.height
            ) {
              if (mesh.material.uniforms.uTexture.value) {
                mesh.material.uniforms.uTexture.value.dispose();
              }
              mesh.material.uniforms.uTexture.value = new THREE.DataTexture(
                null,
                results[0].data.width,
                results[0].data.height,
                THREE.RGBAFormat,
                THREE.UnsignedByteType,
                THREE.UVMapping,
                THREE.ClampToEdgeWrapping,
                THREE.ClampToEdgeWrapping,
                THREE.LinearMipMapNearestFilter,
                THREE.LinearMipMapNearestFilter,
                1
              );
            }
            mesh.material.uniforms.uTexture.value.image.data = new Uint8Array(results[0].arrayBuffer);
            mesh.material.uniforms.uTexture.value.needsUpdate = true;

            if (
              !mesh.material.uniforms.uContent.value ||
              mesh.material.uniforms.uContent.value.width !== results[1].data.width ||
              mesh.material.uniforms.uContent.value.height !== results[1].data.height
            ) {
              if (mesh.material.uniforms.uContent.value) {
                mesh.material.uniforms.uContent.value.dispose();
              }
              mesh.material.uniforms.uContent.value = new THREE.DataTexture(
                null,
                results[1].data.width,
                results[1].data.height,
                THREE.RGBAFormat,
                THREE.UnsignedByteType,
                THREE.UVMapping,
                THREE.ClampToEdgeWrapping,
                THREE.ClampToEdgeWrapping,
                THREE.LinearMipMapNearestFilter,
                THREE.LinearMipMapNearestFilter,
                1
              );
            }
            mesh.material.uniforms.uContent.value.image.data = new Uint8Array(results[1].arrayBuffer);
            // mesh.material.uniforms.uContent.value.needsUpdate = true; // XXX
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

window.ScreenQuad = (() => {

	var defaultQuad = new THREE.PlaneBufferGeometry(2,2,1,1);

	var defaultVertexShader = [

		"varying vec2 vUv;",
		"void main(){",
			"vUv = uv;",
			"gl_Position = vec4(position.xy, 1., 1.);",
		"}",

	].join("\n");

	var defaultFragmentShader = [
		
		`varying vec2 vUv;
    uniform float numTextures;
		uniform sampler2D uTexture1;
		uniform sampler2D uTexture2;
    uniform sampler2D uDepth1;
		uniform sampler2D uDepth2;
		void main() {
      if (numTextures <= 1.0) {
        gl_FragColor = texture2D(uTexture1, vUv);
      } else {
        float depth1 = texture2D(uDepth1, vUv).r;
        float depth2 = texture2D(uDepth2, vUv).r;
        if (depth2 > 0.0 && depth2 <= depth1) {
          gl_FragColor = texture2D(uTexture2, vUv);
        } else {
          vec4 menuColor = texture2D(uTexture1, vUv);
          if (menuColor.a >= 0.9) {
            gl_FragColor = menuColor;
          } else {
            gl_FragColor = texture2D(uTexture2, vUv);
          }
        }
      }
		}`

	].join("\n");


	function ScreenQuad( params ){

		params = params || {};

		THREE.Mesh.apply( this, [ defaultQuad , new THREE.ShaderMaterial({

			uniforms:{
        numTextures: {
          type: 'f',
					value: undefined !== params.numTextures ? params.numTextures : 1
        },
				uTexture1: {
					type:'t',
					value: undefined !== params.texture1 ? params.texture1 : null
				},
        uTexture2: {
					type:'t',
					value: undefined !== params.texture2 ? params.texture2 : null
				},
        uDepth1: {
					type:'t',
					value: undefined !== params.depth1 ? params.depth1 : null
				},
        uDepth2: {
					type:'t',
					value: undefined !== params.depth2 ? params.depth2 : null
				},
			},

			vertexShader: defaultVertexShader,

			fragmentShader: params.fragmentShader ? params.fragmentShader : defaultFragmentShader,

			depthWrite: false,

		})]);

		this.frustumCulled = false;

		this.renderOrder = -1;

		//end mesh setup

		// console.log( this , this.width , this.height );

	}

	ScreenQuad.prototype = Object.create( THREE.Mesh.prototype );

	ScreenQuad.constructor = ScreenQuad;

	return ScreenQuad

})();

window.ScreenQuad = (() => {

	var defaultQuad = new THREE.PlaneBufferGeometry(2,2,1,1);

	var defaultVertexShader = [

		"varying vec2 vUv;",
		"void main(){",
			"vUv = uv;",
			"gl_Position = vec4(position.xy, 0., 1.);",
		"}",

	].join("\n");

	var defaultFragmentShader = [
		
		`varying vec2 vUv;
		uniform sampler2D uTexture1;
    uniform sampler2D uDepth1;
		void main() {
      gl_FragColor = texture2D(uTexture1, vUv);
      gl_FragDepth = texture2D(uDepth1, vUv).r;
		}`

	].join("\n");


	function ScreenQuad( params ){

		params = params || {};

		THREE.Mesh.apply( this, [ defaultQuad , new THREE.ShaderMaterial({

			uniforms:{
				uTexture1: {
					type:'t',
					value: undefined !== params.texture1 ? params.texture1 : null
				},
        uDepth1: {
					type:'t',
					value: undefined !== params.depth1 ? params.depth1 : null
				},
			},

			vertexShader: defaultVertexShader,

			fragmentShader: params.fragmentShader ? params.fragmentShader : defaultFragmentShader,

			// depthWrite: false,

		})]);

		this.frustumCulled = false;

		// this.renderOrder = -1;

		//end mesh setup

		// console.log( this , this.width , this.height );

	}

	ScreenQuad.prototype = Object.create( THREE.Mesh.prototype );

	ScreenQuad.constructor = ScreenQuad;

	return ScreenQuad

})();

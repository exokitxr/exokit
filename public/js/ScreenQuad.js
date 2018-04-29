window.ScreenQuad = (() => {

	var defaultQuad = new THREE.PlaneBufferGeometry(2,2,1,1);

	var defaultVertexShader = [

		"uniform vec4 uSize;",
		"uniform vec2 uSide;",
		"varying vec2 vUv;",
		"void main(){",
			"vUv = uv;",
			"vec2 transformed = position.xy * uSize.xy;",
			// "transformed += vec2( -1. , 1. ) - vec2( - uSize.x , uSize.y );",
			"transformed += uSide - uSide * vec2( uSize.x , uSize.y );",
			"transformed += uSide * vec2( - uSize.w , - uSize.z ) * 2.;",
			"gl_Position = vec4( transformed , 1. , 1. );",
		"}",

	].join("\n");

	var defaultFragmentShader = [
		
		"varying vec2 vUv;",
		"uniform sampler2D uTexture;",
		"void main(){",
			"#ifndef debug",
			"gl_FragColor = texture2D( uTexture , vUv );",
			"#else",
			"gl_FragColor = vec4( vUv , 0. , 1. );",
			'#endif',
		"}"

	].join("\n");


	function ScreenQuad( params ){

		params = params || {};

		var debug = undefined !== params.debug ? params.debug : false;

		THREE.Mesh.apply( this, [ defaultQuad , new THREE.ShaderMaterial({

			uniforms:{
				uTexture:{
					type:'t',
					value: undefined !== params.texture ? params.texture : null
				},
				uSize:{
					type:'v4',
					value:new THREE.Vector4(1,1,0,0)
				},
				uSide:{
					type:'v2',
					value: new THREE.Vector2(1,1)
				}
			},

			vertexShader: defaultVertexShader,

			fragmentShader: params.fragmentShader ? params.fragmentShader : defaultFragmentShader,

			depthWrite: false,

			defines: { debug: debug }

		})]);

		this.frustumCulled = false;

		this.renderOrder = -1;

		//end mesh setup

		this._debug = debug;

		this._pixels = [false,false,false,false,false,false]; //w h t l b r

		this._pairs = {
			topBottom: ['top','bottom'],
			leftRight: ['left', 'right'] 
		};

		//resolve the pairs
		for( var p in this._pairs )
			this._initPair( params , this._pairs[p] );

		/*this.top = undefined !== params.top ? params.top : 0;

		this.left = undefined !== params.left ? params.left : 0;

		this.bottom = undefined !== params.bottom ? params.bottom : false;

		this.right = undefined !== params.right ? params.right : false;*/

		this.width = undefined !== params.width ? params.width : 1;

		this.height = undefined !== params.height ? params.height : 1;

		//cleanup
		this._componentSetters = [
			this.setWidth,
			this.setHeight,
			this.setTop,
			this.setLeft,
			this.setBottom,
			this.setRight
		];

		this._components = [
			'width',
			'height',
			'top',
			'left',
			'bottom',
			'right'
		];

		console.log( this , this.width , this.height );

		this.screenSize = new THREE.Vector2( 1 , 1 );
			
		this.setSize( this.width , this.height );

		if( this.top !== null ) this.setTop( this.top );
		else this.setBottom( this.bottom );

		if( this.left !== null ) this.setLeft( this.left );
		else this.setRight( this.right );

	}

	ScreenQuad.prototype = Object.create( THREE.Mesh.prototype );

	ScreenQuad.constructor = ScreenQuad;

	ScreenQuad.prototype._initPair = function( params , pair ){

		console.log( params );

		if( undefined !== params[ pair[0] ] || undefined !== params[ pair[1] ] ){

			this[ pair[0] ] = undefined === params[ pair[0] ] ? null : params[ pair[0] ]; //top was provided, write top, | not provided but bottom was write null 
			
			this[ pair[1] ] = this[ pair[0] ] !== null ? null : params[ pair[1] ];//top is not null top takes precedence

		} else {

			this[ pair[0] ] = 0;

			this[ pair[1] ] = null;

		}

	}

	ScreenQuad.prototype.setScreenSize = function( width , height ){

		this.screenSize.set( width , height );

		var that = this;
		
		this._pixels.forEach( function(p,pi){

			//if a component is set in pixels, update the uniform 
			if ( p ) that._componentSetters[ pi ].call(that , that[ that._components[pi] ] );  
			
		});

	}
	ScreenQuad.prototype.setSize = function( width , height ){

		this.setWidth( width );
		
		this.setHeight( height );

	};

	ScreenQuad.prototype.setWidth = function( v ) {

		this.width = v;

		if( isNaN( v ) ){

			this.material.uniforms.uSize.value.x = parseInt( v ) / this.screenSize.x;

			this._pixels[0] = true;

		} else {

			this.material.uniforms.uSize.value.x = v;

			this._pixels[0] = false;

		}

	};

	ScreenQuad.prototype.setHeight = function( v ){

		this.height = v;

		if( isNaN( v ) ){

			this.material.uniforms.uSize.value.y = parseInt( v ) / this.screenSize.y;

			this._pixels[1] = true;

		} else {

			this.material.uniforms.uSize.value.y = v;

			this._pixels[1] = false;

		}

	};

	ScreenQuad.prototype.setTop = function( v ) {

		if( null === v ) {

			return; //hack for navigation, the loop up calls these
		
		}
		this.top = v;

		this.bottom = null; //clean this up make one function and structure

		this.material.uniforms.uSide.value.y = 1;

		if( isNaN( v ) ){ 	//if its not a number

			this.material.uniforms.uSize.value.z = parseInt( v ) / this.screenSize.y; //pixels to percentage

			this._pixels[2] = true;													  //this value is in pixels

		} else {

			this.material.uniforms.uSize.value.z = v;								  //just percentage uniform

			this._pixels[2] = false;												  //this value is not in pixels

		}

	}

	ScreenQuad.prototype.setLeft = function( v ){

		if( null === v ) return;

		this.left = v;

		this.right = null;

		this.material.uniforms.uSide.value.x = -1;

		if( isNaN( v ) ){

			this.material.uniforms.uSize.value.w = parseInt( v ) / this.screenSize.x;

			this._pixels[3] = true;

		} else {

			this.material.uniforms.uSize.value.w = v;

			this._pixels[3] = false;

		}

	}

	ScreenQuad.prototype.setBottom = function( v ){

		if( null === v ) return;

		this.bottom = v;

		this.top = null;

		this.material.uniforms.uSide.value.y = -1;

		if( isNaN( v ) ){

			this.material.uniforms.uSize.value.z = parseInt( v ) / this.screenSize.y;

			this._pixels[4] = true;

		} else {

			this.material.uniforms.uSize.value.z = v;

			this._pixels[4] = false;

		}

	}

	ScreenQuad.prototype.setRight = function( v ){

		if( null === v ) return;

		this.right = v;

		this.left = null

		this.material.uniforms.uSide.value.x = 1;

		if( isNaN( v ) ){

			this.material.uniforms.uSize.value.w = parseInt( v ) / this.screenSize.x;


			this._pixels[5] = true;

		} else {

			this.material.uniforms.uSize.value.w = v;

			this._pixels[5] = false;

		}

	}

	return ScreenQuad

})();

// MatCap-style image rendered on a sphere
// modify sphere UVs instead of using a ShaderMaterial

var camera, scene, renderer;
var image;

init();
animate();

function init() {

	info = document.createElement( 'div' );
	info.style.position = 'absolute';
	info.style.top = '30px';
	info.style.width = '100%';
	info.style.textAlign = 'center';
	info.style.color = '#fff';
	info.style.fontWeight = 'bold';
	info.style.backgroundColor = 'transparent';
	info.style.zIndex = '1';
	info.style.fontFamily = 'Monospace';
	info.innerHTML = 'Drag mouse to rotate camera; scroll to zoom';
	document.body.appendChild( info );

    renderer = new THREE.WebGLRenderer();
	renderer.setSize( window.innerWidth, window.innerHeight );
	document.body.appendChild( renderer.domElement );

	scene = new THREE.Scene();
	
	camera = new THREE.PerspectiveCamera( 40, window.innerWidth / window.innerHeight, 1, 1000 );
	camera.position.set( 0, 0, 0 );
    scene.add( camera ); // since light is child of camera

	controls = new THREE.OrbitControls( camera );
	controls.minDistance = 75;
	controls.maxDistance = 200;
    controls.enablePan = false;

	scene.add( new THREE.AmbientLight( 0xffffff, 0.8 ) );
	
	var light = new THREE.PointLight( 0xffffff, 1 );
	camera.add( light );

    image = document.createElement( 'img' );
    document.body.appendChild( image );

    var texture = new THREE.Texture( image );
    image.addEventListener( 'load', function ( event ) { texture.needsUpdate = true; } );

	var material = new THREE.MeshPhongMaterial( {
		color: 0xffffff, 
		specular: 0x050505,
		shininess: 50,
		map: texture
	} );

	var geometry = new THREE.SphereGeometry( 80, 32, 16 );

    skyBox = new THREE.Mesh(geometry, material);  
    skyBox.scale.set(-1, 1, 1);  
    skyBox.eulerOrder = 'XZY';  
    skyBox.renderDepth = 1000.0;  
    scene.add(skyBox);  


    //mesh = new THREE.Mesh( geometry, material );
	//scene.add( mesh );

}

function animate() {

	requestAnimationFrame( animate );

	//controls.update(); // not required here

	render();

}

function render() {

	renderer.render( scene, camera );

}

image.src = '../images/rendered_0.png';


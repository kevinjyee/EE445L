// MAIN LOOP
var main = function(){
	var now = Date.now();
	var delta = now - then;

	update(delta / 1000);
	render();

	then = now;

	//REQUEST TO DO THIS GAIN
	requestAnimationFrame(main);
};
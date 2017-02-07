var express = require('express');
var app     = express();
var http    = require('http').Server(app);
var jsonfile = require('jsonfile');

/* CONFIG for res.render to ejs Front End Files */
/* -------------------------------------------- */
var configFile = './config/config.json';
var cameraOptions = {};

jsonfile.readFile(configFile, function(err, obj) {
	if (err) console.err(err);
	cameraOptions = {
		width       : obj.camera.width,
		height      : obj.camera.height,
		mode        : obj.camera.mode,
		awb         : obj.camera.awb,
		output      : obj.camera.output,
		q           : obj.camera.q,
		rot         : obj.camera.rot,
		nopreview   : obj.camera.nopreview,
		timeout     : obj.camera.timeout,
		timelapse   : obj.camera.timelapse,
		th          : obj.camera.th
	};
	
	var camera = new require("raspicam")(cameraOptions);
	camera.on("stop", function(){
		console.log("Stop Camera");
	})
	
	.on("start", function (){
		console.log("Start Camera");
	})
	
	.on("exit", function() {
		camera.stop();
		console.log('Restarting camera...')
		camera.start()
	});
	camera.start();
});
	
app.use(express.static(__dirname + '/images'));
http.listen(8888, function(){
    console.log('Running...');
});
 
app.get('/', function(req, res)
{
	console.log('get / ');
    res.sendFile(__dirname + '/images/camera.jpg');
});

camera.on("exit", function()
{
    camera.stop();
    console.log('Restarting camera...')
    camera.start()
});

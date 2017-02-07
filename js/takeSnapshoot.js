var express = require('express');
var app     = express();
var http    = require('http').Server(app);
var jsonfile = require('jsonfile');
var moment = require('moment');
moment = moment().format('YYYY-MM-DDTHH:mm:ss\\Z')

/* CONFIG for res.render to ejs Front End Files */
/* -------------------------------------------- */
var camera = new require("raspicam");
var configFile = './config/config.json';
var cameraOptions = {};
var output = "snapshoot/"+"IMG_"+moment+".jpg";

jsonfile.readFile(configFile, function(err, obj) {
	if (err) console.err(err);
	cameraOptions = {
		width       : obj.camera.width,
		height      : obj.camera.height,
		mode        : obj.camera.mode,
		awb         : obj.camera.awb,
		output      : output,
		q           : obj.camera.q,
		rot         : obj.camera.rot,
		nopreview   : obj.camera.nopreview,
		timeout     : obj.camera.timeout,
		timelapse   : obj.camera.timelapse,
		th          : obj.camera.th
	};

	camera = new require("raspicam")(cameraOptions);
	camera.start();
	camera.on("stop", function(){
		console.log("Stop Camera");
	});
	camera.on("start", function (){
		console.log("Start Camera");
	});
	camera.on("exit", function() {
		camera.stop();
	});
});

/*	
app.use(express.static(__dirname + '/images'));
http.listen(8888, function(){
    console.log('Running...');
});
 
app.get('/', function(req, res)
{
	console.log('get / ');
    res.sendFile(__dirname + '/images/camera.jpg');
});
*/

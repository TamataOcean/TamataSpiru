var express = require('express');
var app     = express();
var http    = require('http').Server(app);

var moment = require('moment');
moment = moment().format('YYYY-MM-DDTHH:mm\\Z');

var jsonfile = require('jsonfile');

/* CONFIG for res.render to ejs Front End Files */
/* -------------------------------------------- */
var camera = new require("raspicam");
var configCamera = './config/camera.json';
var snapshootFile = './config/snapshootFile.json';

var cameraOptions = {};
var output = "snapshoot/"+"IMG_"+moment+".jpg";
var fs = require('fs'); 
 
jsonfile.readFile(configCamera, function(err, obj) {
	if (err) console.err(err);
	cameraOptions = {
		width       : obj.width,
		height      : obj.height,
		mode        : obj.mode,
		awb         : obj.awb,
		output      : output,
		q           : obj.q,
		rot         : obj.rot,
		nopreview   : obj.nopreview,
		timeout     : obj.timeout,
		timelapse   : obj.timelapse,
		th          : obj.th
	};

	camera = new require("raspicam")(cameraOptions);
	camera.start();
	/* Start Event */
	camera.on("start", function (){
		console.log("Start Camera");
	});
	
	/* Exit Event */ 
	camera.on("exit", function() {
		camera.stop();
	});
	
	camera.on("stop", function(){
		console.log("Stop Camera");
		var results = [];
		fs.readdir("snapshoot/", function(err, list) {
			if (err) return done(err);
			var i=0;
			var pending = list.length;
			var result = '{"images":[';
			var lastone = '';
			if (!pending) return done(null, results);
			var monJSON ="";
			//console.log("List Length :"+list.length);
			list.forEach(function(file) {
				if (i === list.length-1) {
					lastone = file;
					lastindex = i;
					result += '{"img":"'+file+'",';
					result += '"index":"'+i+'"}';
				}
				else {
					result += '{"img":"'+file+'",';
					result += '"index":"'+i+'"},';
				}
				i++;
			});
			result += '],';
			result += '"lastone":"'+lastone+'"}';
			console.log(moment+" - SnapShoot Request : "+ JSON.stringify(JSON.parse(result)));
			jsonfile.writeFile(snapshootFile,JSON.parse(result), function(err) { if (err) throw err});
		});	
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

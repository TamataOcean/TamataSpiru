/*-------- Tamata Lib to get SnapShoot -----------------*/
/* - Taking picture from camera							*/
/* - Recording file list onto config/camera.json file 	*/
/*------------------------------------------------------*/
var express = require('express');
var app     = express();
var http    = require('http').Server(app);
var fs 		= require('fs'); 
var moment 	= require('moment');
moment = moment().format('YYYY-MM-DDTHH:mm\\Z');
var jsonfile = require('jsonfile');

/* CONFIG for res.render to ejs Front End Files */
/* -------------------------------------------- */
var camera = new require("raspicam");
var configCamera = './config/camera.json';
var snapshootFile = './config/snapshootFile.json';
var cameraOptions = {};
var output = "snapshoot/"+"IMG_"+moment+".jpg";

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
	
	/* On Stop, update snap list onto json file */
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

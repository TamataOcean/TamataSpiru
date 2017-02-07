var express = require('express');
var app     = express();
var http    = require('http').Server(app);

/* CONFIG for res.render to ejs Front End Files */
/* -------------------------------------------- */
var configFile = './config/config.json';

app.use(express.static(__dirname + '/images'));
http.listen(8888, function(){
    console.log('Running...');
});
 
var cameraOptions = {
    width       : 1024,
    height      : 920,
    mode        : "timelapse",
    awb         : 'cloud',
    output      : 'images/camera.jpg',
    q           : 50,
    rot         : 180,
    nopreview   : true,
    timeout     : 1000,
    timelapse   : 999,
    th          : "0:0:0"
};
 

 
var camera = new require("raspicam")(cameraOptions);
camera.start();
 
app.get('/', function(req, res)
{
    res.sendFile(__dirname + '/images/camera.jpg');
});

camera.on("exit", function()
{
    camera.stop();
    console.log('Restarting camera...')
    camera.start()
});

/*!
 * TamataSpiru
 * Copyright(c) 2016-2017 Romain Tourte
 * Contact : contact@tamataocean.com
 * Version 0.0.2 - Adding to GitHub + Web Interface Applications.
 * MIT Licensed - 
 */
  
var fs = require('fs')
//var tamatalib = require('./lib/tamatasensors')

var jsonfile = require('jsonfile')
var moment = require('moment')
var lastupdate = moment().format('YYYY-MM-DDTHH:mm:ss\\Z')
var mqtt = require('mqtt')
/* MQTT
topic : $aws/things/tamatataRASPI/shadow/update/delta */
var mqttServer = '172.16.10.128'
var topicTamata = 'tamataraspi/spiru/update';
var topicDelta = 'tamataraspi/spiru/update/delta';

/* CONFIG for res.render to ejs Front End Files */
/* -------------------------------------------- */
var configFile = './config/config.json';
var configSnapshoot = './config/snapshootFile.json';
/* Front End Files */
var ejs_index = 'indexW3.ejs';
var ejs_dashboard = 'dashboardW3.ejs';
var ejs_sched = 'schedW3.ejs';
var ejs_alert = 'alertsW3.ejs';
var ejs_remote = 'remoteW3.ejs';

var sensorsCount = 0;

// Load the full build.
var _ = require('lodash');

/* ------------ I2C ------------ NOT YET... 
var i2c = require('i2c');
var address = 0x04;
var wire = new i2c(address, {device: '/dev/i2c-1'});
 
/* ------------ Express.JS ------------ */  
/* ------------------------------------ */
var express = require('express');
var session = require('cookie-session'); // Charge le middleware de sessions
var bodyParser = require('body-parser'); // Charge le middleware de gestion des paramètres
var urlencodedParser = bodyParser.urlencoded({ extended: false });
var app = express();

/* Config JSON indent mode */
jsonfile.spaces = 4;
/* Using sessions */
app.use(session({secret: 'tamataSpiru'}))

/* Static files declared */
.use(express.static(__dirname + '/img'))
.use(express.static(__dirname + '/fonts'))
.use(express.static(__dirname + '/css'))
.use(express.static(__dirname + '/js'))
.use(express.static(__dirname + '/snapshoot'))

.use(bodyParser.urlencoded({
    extended: true
}))

.use(bodyParser.json())

/* --------------------------- Index print ------------------------ */
/* ---------------------------------------------------------------- */
.get('/', function(req, res) {
	jsonfile.readFile(configFile, function(err, obj) {
		if (err) throw err;
		//Check Params 
		res.render(ejs_index, {
			title : "TamataSpiru - Home",
			lastupdate : lastupdate,
			manual : obj.actors.manual,
			heat : {
				mc_temperature : obj.sensors.temperature.mc,
				ext_temperature :  obj.sensors.temperature.ext,
				target_temperature : obj.actors.heat.target_temperature,
				check_power : obj.actors.heat.state,			//TODO : Check Power consume
				check_temp : true 			//TODO : Check if  14 << temperature MC << 41 & ...
				},
			light : {
				light_UV : obj.sensors.light.uv,
				light_IR : obj.sensors.light.ir,
				Light_Vis : obj.sensors.light.vis,
				check_power : obj.actors.light.state,	//TODO : + Power consume & efficiency... 
				check_light : true			//TODO : + analyse spectrum & data
				},
			rgb : {
				rgb_red : obj.sensors.color.r,
				rgb_green : obj.sensors.color.g,
				rgb_blue : obj.sensors.color.b,
				lux : obj.sensors.color.lux,
				temp : obj.sensors.color.temp,
				colorHex : obj.sensors.color.colorHex,
				check_rgb : true			//TODO : + Check if (Blue << Green << Red)
			},
			check_power_move : obj.actors.bubler.state			//TODO : + Check bubler consume.
		});
	})
})

/* --------- Remote Control Refresh Data on Index Page ------------- */
/* ----------------------------------------------------------------- */
.get('/refreshdata', function(req, res) {
	jsonfile.readFile(configFile, function(err, obj){
		var clientRasPi = mqtt.connect({host:'tamataraspi8go.local',port:1883})
		/* JSON */
		clientRasPi.on('connect',function(){
			clientRasPi.subscribe('tamataraspi/spiru/update')
			console.log('Connected to Topic TamataSpiru')
		})

		clientRasPi.on('message', function(topic, message) {
			console.log('get Message from TamataSpiru : %s', message)		
			//JSON Analyse 
			var jsonCool = JSON.parse(message);
			
			obj.actors.heat.state = jsonCool.state.reported.onHeat;
			obj.actors.heat.target_temperature = jsonCool.state.reported.HeatTARGET;
			obj.actors.light.state = jsonCool.state.reported.onLight;
			obj.actors.bubler.state = jsonCool.state.reported.onBubler;
			obj.actors.manual = jsonCool.state.reported.MANUAL;
			obj.sensors.temperature.mc = jsonCool.state.reported.Temp1;
			obj.sensors.temperature.ext = jsonCool.state.reported.Temp;
			obj.sensors.light.ir = parseFloat(jsonCool.state.reported.IR);
			obj.sensors.light.uv = parseFloat(jsonCool.state.reported.UV);
			obj.sensors.light.vis = parseFloat(jsonCool.state.reported.Vis);
			obj.sensors.color.r = jsonCool.state.reported.r;
			obj.sensors.color.g = jsonCool.state.reported.g;
			obj.sensors.color.b = jsonCool.state.reported.b;
			obj.sensors.color.lux = jsonCool.state.reported.lux;
			obj.sensors.color.temp = parseFloat(jsonCool.state.reported.colorTemp);
			obj.sensors.color.colorHex = jsonCool.state.reported.colorHex;
			
			console.log('JSON stringify : '+JSON.stringify(obj));
			clientRasPi.end();
			jsonfile.writeFile(configFile, obj, function(err) {console.error(err)});
			res.redirect('/');
		})
	});
})

/* ----------------------------- Dashboard ------------------------ */
/* ---------------------------------------------------------------- */
.get('/stats', function(req, res) { 
    	jsonfile.readFile(configFile, function(err, obj) {
			if (err) console.err(err);
			res.render(ejs_dashboard, {
			title : 'TamataSpiru Dashboard',
			//iframeDashboard : 'https://search-mosquitto-k34joo6mwdg7ynzkts6epsxuqy.eu-west-1.es.amazonaws.com/_plugin/kibana/?embed&#/dashboard/Tamata-Spiru-DashBoard?_a=(filters:!(),panels:!((col:1,id:Tamata-Color,row:4,size_x:4,size_y:3,type:visualization),(col:8,id:Tamata-Atmospheric-Pressure,row:1,size_x:3,size_y:3,type:visualization),(col:5,id:Tamata-Humidity-ampersand-Moisture,row:1,size_x:3,size_y:3,type:visualization),(col:8,id:Tamata-Light-Conditions,row:4,size_x:3,size_y:3,type:visualization),(col:5,id:Tamata-UV,row:4,size_x:3,size_y:3,type:visualization),(col:1,id:Tamata-Temperature_OK,row:1,size_x:4,size_y:3,type:visualization)),query:(query_string:(analyze_wildcard:!t,query:\'id%3D!\'TamataSpiru!\'\')),title:\'Tamata%20Spiru%20DashBoard\')&_g=(refreshInterval:(display:Off,pause:!f,section:0,value:0),time:(from:now%2Fd,mode:quick,to:now%2Fd))'
			iframeDashboard : 'https://search-mosquitto-k34joo6mwdg7ynzkts6epsxuqy.eu-west-1.es.amazonaws.com/_plugin/kibana/#/dashboard/Tamata-Spiru-DashBoard?_a=(filters:!(),panels:!((col:1,id:Tamata-Color,row:4,size_x:4,size_y:3,type:visualization),(col:9,id:Tamata-Atmospheric-Pressure,row:1,size_x:3,size_y:3,type:visualization),(col:6,id:Tamata-Humidity-ampersand-Moisture,row:1,size_x:3,size_y:3,type:visualization),(col:8,id:Tamata-Light-Conditions,row:4,size_x:4,size_y:3,type:visualization),(col:5,id:Tamata-UV,row:4,size_x:3,size_y:3,type:visualization),(col:1,id:Tamata-Temperature_OK,row:1,size_x:5,size_y:3,type:visualization)),query:(query_string:(analyze_wildcard:!t,query:%27id%3D!%27TamataFarm!%27%27)),title:%27Tamata%20Spiru%20DashBoard%27)&_g=(refreshInterval:(display:Off,pause:!f,section:0,value:0),time:(from:now%2FM,mode:quick,to:now%2FM))'
		})
	});
})

/* ----------------------------- Scheduling ----------------------- */
/* ---------------------------------------------------------------- */
.get('/sched', function(req, res) {
	jsonfile.readFile(configFile, function(err, obj) {
		if (err) throw err;
		res.render(ejs_sched, {
			title : 'TamataSpiru Sched',
			heat : {
				pwm : obj.actors.heat.pwm,
				mc_temperature : 35,
				target_temperature : obj.actors.heat.target_temperature,
				ext_temperature : 23,
				check_power_temp : true,
				check_temp : true, 
				sched : {
					timer_on : obj.actors.heat.sched.timer_on,
					timer_off : obj.actors.heat.sched.timer_off
					}
				},
			light : {
				pwm : obj.actors.light.pwm,	
				check_light : true,			
				check_power : true, 
				sched : {
					timer_on : obj.actors.light.sched.timer_on,
					timer_off : obj.actors.light.sched.timer_off
					}
			},
			bubler : {
				pwm : obj.actors.bubler.pwm,				
				check_power : true, 
				sched : {
					timer_on : obj.actors.bubler.sched.timer_on,
					timer_off : obj.actors.bubler.sched.timer_off
					}
			}
			
		});
	})	
})
.post('/save_sched', function(req, res, next){
	jsonfile.readFile(configFile, function(err, obj){
		if (err) throw err;
		// Temperature Paramaters
		obj.actors.heat.target_temperature = parseFloat(req.body.target_temperature);
		obj.actors.heat.sched.timer_on = req.body.timer_on_temperature;
		obj.actors.heat.sched.timer_off = req.body.timer_off_temperature;
		
		// Light Paramaters
		obj.actors.light.pwm = req.body.intensity_light;
		obj.actors.light.sched.timer_on = req.body.timer_on_light;
		obj.actors.light.sched.timer_off = req.body.timer_off_light;
		
		// Bubler Parameters
		obj.actors.bubler.pwm = req.body.intensity_bubler;
		obj.actors.bubler.sched.timer_on = req.body.timer_on_bubler;
		obj.actors.bubler.sched.timer_off = req.body.timer_off_bubler;
		
		jsonfile.writeFile(configFile, obj, function(err) {console.error(err)});
		console.log('Sched - Update Config.JSON File'+ JSON.stringify(obj)); 
		
		// SEND Event to CoolBoard 
		var client = mqtt.connect({host:'tamataraspi8go.local', port:1883})
		var objControl = 'Scheduling';
		var Hour = new Date(obj.actors.heat.sched.timer_on);
		console.log("Date = "+Hour);
		var desired = "{\"state\":{";
		desired += "\"INTERVAL\":"+10+",";
		desired += "\"HeatTARGET\":"+obj.actors.heat.target_temperature+",";
		desired += "\"HeatONhour\":"+String(obj.actors.heat.sched.timer_on).substr(0,2)+",";
		desired += "\"HeatOFFhour\":"+String(obj.actors.heat.sched.timer_off).substr(0,2)+",";
		desired += "\"HeatONminute\":"+String(obj.actors.heat.sched.timer_on).substr(3,4)+",";
		desired += "\"HeatOFFminute\":"+String(obj.actors.heat.sched.timer_off).substr(3,4)+",";
		desired += "\"LightONhour\":"+String(obj.actors.light.sched.timer_on).substr(0,2)+",";
		desired += "\"LightOFFhour\":"+String(obj.actors.light.sched.timer_off).substr(0,2)+",";
		desired += "\"LightONminute\":"+String(obj.actors.light.sched.timer_on).substr(3,4)+",";
		desired += "\"LightOFFminute\":"+String(obj.actors.light.sched.timer_off).substr(3,4)+",";
		desired += "\"BublerONhour\":"+String(obj.actors.bubler.sched.timer_on).substr(0,2)+",";
		desired += "\"BublerOFFhour\":"+String(obj.actors.bubler.sched.timer_off).substr(0,2)+",";
		desired += "\"BublerONminute\":"+String(obj.actors.bubler.sched.timer_on).substr(3,4)+",";
		desired += "\"BublerOFFminute\":"+String(obj.actors.bubler.sched.timer_off).substr(3,4)+"";
		desired += "}}";
		
		client.on('connect',function() {
			client.publish(topicDelta,desired)
			console.log("Switch request for : "+objControl + "/" +desired)
			client.end()
		})
		
		res.redirect('/sched');
	})
})

/* --------------------------- Alerts ----------------------------- */
/* ---------------------------------------------------------------- */
.get('/alert', function(req, res) { 
	jsonfile.readFile(configFile, function(err, obj){
		if (err) throw err;
		res.render(ejs_alert, {
			title: 'TamataSpiru Alerts',
			alert_email:obj.alerts.email,
			alert_tel:obj.alerts.tel,
			alerts:{
				message:obj.alerts.message,
				temperature:{
					max: obj.alerts.temperature.max,
					min: obj.alerts.temperature.min
				},
				light:{
					uv: obj.alerts.light.alertUV.max,
					ir: obj.alerts.light.alertIR.max
				}
			},
		});
	});
})
.post('/save_alerts', function(req, res, next){
	// req.body object has your form values
	jsonfile.readFile(configFile, function(err, obj){
		if (err) throw err;
		obj.alerts.message = "Last Update : "+String(moment().format('HH:mm:ss'));
		obj.alerts.temperature.min = parseFloat(req.body.temperature_alert_min);
		obj.alerts.temperature.max = parseFloat(req.body.temperature_alert_max);
		
		obj.alerts.light.alertUV.max = parseFloat(req.body.light_alert_uv);
		obj.alerts.light.alertIR.max = parseFloat(req.body.light_alert_ir);
		
		obj.alerts.email = req.body.alert_email;
		obj.alerts.tel = req.body.alert_tel;
		
		console.log('Alert - Update Config.JSON '+ JSON.stringify(obj)); // + SEND Event to Update Arduino
		jsonfile.writeFile(configFile, obj, function(err) {console.error(err)});
		res.redirect('/alert');
	})   
})

/* ----------------------------- Remote ------------------------ */
/* ---------------------------------------------------------------- */
.get('/system', function(req, res) { 
/*
	jsonfile.readFile(configFile, function(err, obj){
		if (err) throw err;
		res.render(ejs_system, {
			title : 'TamataSpiru System Environment',
			Interval : obj.
			
		}
	)};
	*/
	res.redirect('/');
})

/* ----------------------------- Remote ------------------------ */
/* ---------------------------------------------------------------- */
.get('/remote', function(req, res) { 
    jsonfile.readFile(configFile, function(err, obj){
		if (err) throw err;
		jsonfile.readFile("./config/snapshootFile.json", function(err, snapFile){
			if (err) throw err;
			var snapshootTab = snapFile;
			//console.log(snapshootTab);
			res.render(ejs_remote, {
				title : 'TamataSpiru Remote Control',
				check_power_move : true,
				snapshoot : snapshootTab,
				lastone : snapFile.lastone,
				temperature : {
					state : obj.actors.heat.state,
					check_temp : true 
				},
				light : {
						light_UV : 650,
						light_IR : 780,
						check_light : true,			//TODO : + analyse spectrum & data
						state : obj.actors.light.state			//TODO : + Power consume & efficiency... 
						},
				bubler : {
						state : obj.actors.bubler.state
				},
				rgb : {
						check_rgb : true
				},
				manual : obj.actors.manual
			});
		});
	});
})


/* --------------------------- Snapshoot -------------------------- */
/* ---------------------------------------------------------------- */
.get('/snapshoot', function(req, res) {
	console.log('snapshoot requested !! ');
	var _exec = require('child_process').exec;
	_exec( 'node ./js/takeSnapshoot.js', function(e){
        console.log( "Photo Snapshoot Return : ");
		res.redirect('/remote');
    });
})

/* --------- Remote Control On/Off (Heat, Bubler, Light ) ---------- */
/* ----------------------------------------------------------------- */
.get('/remoteOrder', function(req, res) { 
	var desired = '';
	lastupdate = moment().format('YYYY-MM-DDTHH:mm:ss\\Z')
	jsonfile.readFile(configFile, function(err, obj){
		if (err) throw err;
		
		var client = mqtt.connect({host:'tamataraspi8go.local', port:1883})
		var objControl = '';
		/* ------- MANUAL MODE ON / OFF -------*/
		if ( req.param('manual') == 'switch') {
			objControl = 'manual';
			if (obj.actors.manual) {
				desired = "{\"state\":{\"MANUAL\":3}}"
			} else {
				desired = "{\"state\":{\"MANUAL\":4}}"
			}
			client.on('connect',function() {
				client.publish(topicDelta,desired)
				console.log("Switch request for : "+objControl + "/" +desired)
				client.end()
			})
			obj.actors.manual = !(obj.actors.manual);
		}
		/* ------- HEAT = Act5 ( On = 4 / Off =  3) ------- */
		else if ( req.param('heat') === 'switch') {
			objControl = 'Heat';
			if (obj.actors.heat.state) {
				desired = "{\"state\":{\"Act5\":3}}"
			} else {
				desired = "{\"state\":{\"Act5\":4}}"
			}
			client.on('connect',function() {
				client.publish(topicDelta,desired)
				console.log("Switch request for : "+objControl + "/" +desired)
				client.end()
			})
			obj.actors.heat.state = !(obj.actors.heat.state);
		} 
		/* ------- BUBLER = Act4 ( On = 4 / Off =  3) ------- */
		else if ( req.param('bubler') === 'switch') {
			objControl = 'bubler';
			if (obj.actors.bubler.state) {
				desired = "{\"state\":{\"Act4\":3}}"
			} else {
				desired = "{\"state\":{\"Act4\":4}}"
			}
			client.on('connect',function() {
				client.publish(topicDelta,desired)
				console.log("Switch request for : "+objControl + "/" +desired)
				client.end()
			})
			obj.actors.bubler.state = !(obj.actors.bubler.state);
		}
		/* ------- LIGHT = Act6 ( On = 4 / Off =  3) ------- */
		else if ( req.param('light') === 'switch') {					
			objControl = 'light';
			if (obj.actors.light.state) {
				desired = "{\"state\":{\"Act6\":3}}"
			} else {
				desired = "{\"state\":{\"Act6\":4}}"
			}
			client.on('connect',function() {
				client.publish(topicDelta,desired)
				client.end()
			})
			obj.actors.light.state = !(obj.actors.light.state);
		}
		else {
			//console.log('Invalid parameter sent : '+req.param);
			res.redirect('/remote');
		}
		console.log(lastupdate+" - Switch request for : "+objControl + "/" +desired)
		//Update State
		jsonfile.writeFile(configFile, obj, function(err) {console.error(err)});
		res.redirect('/remote');
	});
})

/* ---------------------- Unknown Page -----------------------------*/
/* -----------------------------------------------------------------*/
.use(function(req, res, next){
	//console.log('Invalid adress sent !! : '+res);
    res.redirect('/');
});

function finish(){
			console.log('Finish :' + sensorsCount);
			if (sensorsCount === 4 ) {
				jsonfile.writeFile(file,obj, function(err) { if (err) throw err});
				lastupdate = moment().format('HH:mm:ss')
				res.redirect('/');
			}
		}

app.on('connect',function(req,res) {
	console.log('new user arrived');
});
app.listen(8080);
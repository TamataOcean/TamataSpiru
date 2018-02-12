/* Script lauchned automatically to get Data from Broker to send them  to DataBase*/
/* ------------------------------------------------------------------------------ */

let fs = require('fs')
var jsonfile = require('jsonfile')
var moment = require('moment')
var mqtt = require('mqtt')
var winston = require('winston');				//Logger
require('winston-logrotate');
var _ = require('lodash');

/* Config JSON indent mode */
configFile = "../../config/config.json";
jsonfile.spaces = 4;
var mqttTopic = ""; //'tamataraspi/COOL_Spiru/update' 		// TamataFarm Topic 
var mqttServer = ""; // '10.3.141.1';
var mqttAWS ="";
var user = "";
var id = "";

/* Check Time && Logger declaration */
moment = moment().format('YYYY-MM-DDTHH:mm:ss\\Z')

var logger = new (winston.Logger)({
	name : 'tamataFarm_Log',
	levels: {
		'info': 0, 
		'ok': 1,
		'error': 2, 
		'debug':3,
	},
	colors: {
        'info': 'yellow',
        'ok': 'green',
        'error': 'red', 
		'debug': 'cyan'
       },
    transports: [
        new (winston.transports.Console)({level:'info',colorize: true}),
		new (winston.transports.File)({ filename: 'winston.log' })	
    ]
});


/* JSON */
var objJSON = "";
logger.info('TamataSpiru Transfert ('+moment+')')

var file = '/home/bibi/node/log/actionMqttDelta_'+moment+'.json'
var msgCount = 0

//---------------------
//get CoolCoSpiru Board
//---------------------
jsonfile.readFile(configFile, function(err, obj) {
		if (err) throw err;
		mqttTopic = obj.system.mqttTopic + '/update/delta';
		mqttServer = obj.system.mqttServer;
		mqttAWS = obj.system.mqttAWS;
		user = obj.system.user;
		actionMqttDelta();
});


// var jsonDelta =	{"state":{"desired":{ "jetPatck": { "Act3": {"high":[23,200000,6,0]}}}}};
var jsonDelta =	{"state":{"desired":{"CoolBoard":{"logInterval":60}}}};
//var jsonDelta =	{"state":{"desired":{"rtc":{"timeServer":"tamataraspi.local"}}}};
//var jsonDelta =	{"state":{"desired":{"jetPack":{"Act5":{"temporal":0 }}}}};
// var jsonDelta =	{"state":{"desired":{"jetPack":{"Act5":{"high":[25,200000,4,0]}}}}};
// var jsonDelta =	{"state":{"desired":{"jetPack":{ "Act5":{"actif":1,"inverted":0,"temporal":1,"low":[0,0,18,0],"high":[0,0,10,0],"type":["","hour"],"comment":"Light"}}}}};
// var jsonDelta =	{"state":{"desired":{"jetPack":{ "Act3":{"actif":1,"inverted":0,"temporal":0,"low":[17,100000,17,0],"high":[25,200000,4,0],"type":["TempMC","heatSpiru"],"comment":"Heater"} }}}};
//var jsonDelta =	{"state":{"desired":{"jetPack":{ "Act3":{"high":[25,200000,4,0],"low":[17,100000,17,0],"actif":true,"temporal":true,"inverted":false, "type":["TempMC","heatSpiru"],"comment":"Heater"  } }}}};

function actionMqttDelta() {
		logger.info('action : sending Json delta : ...');
		logger.info(jsonDelta);
		var client = mqtt.connect({host:"10.3.141.1", port:1883});
		client.on('connect',function() {	
			client.publish(mqttTopic,JSON.stringify(jsonDelta))
			logger.info("Publish on "+ mqttTopic + "at : " +moment)
			client.end()
		});
}


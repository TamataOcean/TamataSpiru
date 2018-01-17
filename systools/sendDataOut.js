/* Script lauchned automatically to get Data from Broker to send them  to DataBase*/
/* ------------------------------------------------------------------------------ */

let fs = require('fs')
var jsonfile = require('jsonfile')
var moment = require('moment')
var mqtt = require('mqtt')
var winston = require('winston');				//Logger
require('winston-logrotate');

// Load the full build.
var _ = require('lodash');

/* I2C Necessary from Raspberry connected with I2C and Arduino
var i2c = require('i2c')
var address = 0x04;
var wire = new i2c(address, {device: '/dev/i2c-1'});*/

/* MQTT
topic : $aws/things/tamatataRASPI/shadow/update/delta */

/* Config JSON indent mode */
configFile = "/home/bibi/node/config/config.json";
jsonfile.spaces = 4;
var mqttTopic = ""; //'tamataraspi/COOL_Spiru/update' 		// TamataFarm Topic 
var mqttServer = ""; // '10.3.141.1';
var mqttAWS ="";
var user = "";
var id = "";

/* Variables */
var temperature;
var red;
var green;
var blue;

/* Check Time && Logger declaration */
moment = moment().format('YYYY-MM-DDTHH:mm:ss\\Z')
		
/* Controlling Size off Log file */
/*var rotateTransport = require('winston-logrotate').Rotate({
        file: '/home/pi/node/tamataspiru/log/tamataFarm.log', // this path needs to be absolute
        colorize: false,
        timestamp: true,
        json: false,
        max: '100m',
        keep: 5,
        compress: true
});*/

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

var file = '/home/pi/node/tamataspiru/log/ExportTamataFarm_'+moment+'.json'
var msgCount = 0

//---------------------
//get CoolCoSpiru Board
//---------------------
jsonfile.readFile(configFile, function(err, obj) {
		if (err) throw err;
		mqttTopic = obj.system.mqttTopic + '/update';
		mqttServer = obj.system.mqttServer;
		mqttAWS = obj.system.mqttAWS;
		user = obj.system.user;
		getCoolCoSensors();
});

function getCoolCoSensors(){
	var clientRasPi = mqtt.connect({host:mqttServer,port:1883})
	/* JSON */
	clientRasPi.on('connect',function(){
		clientRasPi.subscribe(mqttTopic)
		logger.info('Connected to Topic TamataSpiru')
	})
	
	clientRasPi.on('message', function(topic, message) {
		logger.info('Open topic = %s',topic)		
				
		objJSON = {}
		var jsonCool = JSON.parse(message);
		
		//JSON Analyse 
		for(var exKey in jsonCool.state.reported) {
    		logger.debug("key:"+exKey+", value:"+jsonCool.state.reported[exKey]);
    		_.set(objJSON, "state.reported."+exKey, jsonCool.state.reported[exKey])
		}

		//_.set(objJSON, 'state.reported.lat',"49°33'1029N")
		//_.set(objJSON, 'state.reported.lon',"01°51'3173W")

		logger.info('-----------------------------------------')
		logger.info('JSON stringify : '+JSON.stringify(objJSON))
		logger.debug('Parse message ok')	
		clientRasPi.end()
		msgCount=5;
		finale();
	})
}

function finale()
{
	//Waiting for full execution
	if (msgCount === 5 )
		{
			// Send JSON to MQTT Broker
			logger.info('Finale : Mqtt transfert...')
			var client = mqtt.connect({host:mqttAWS, port:1883})
			client.on('connect',function() {	
				client.publish(mqttTopic,JSON.stringify(objJSON))
				logger.info("Publish on Kibana "+moment)
				client.end()
			})
			// Write Temp File waiting for connection available.
			//jsonfile.writeFile(file,objJSON, function(err) { if (err) throw err});
			msgCount = 0;
		}
}



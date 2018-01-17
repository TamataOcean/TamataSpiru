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
configFile = "/home/pi/node/config/config.json";
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

var jsonDelta =	{"state":{"desired":{"CoolBoard":{"logInterval":60}}}};

function actionMqttDelta() {
		logger.info('action : sending Json delta : ...');
		logger.info(jsonDelta);
		var client = mqtt.connect({host:mqttServer, port:1883});
		client.on('connect',function() {	
			client.publish(mqttTopic,JSON.stringify(jsonDelta))
			logger.info("Publish on "+ mqttTopic + "at : " +moment)
			client.end()
		});
}


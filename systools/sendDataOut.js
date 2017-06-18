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

var configFile = '../config/config.json';
/* Config JSON indent mode */
jsonfile.spaces = 4;
var topicTamata = ""; //'tamataraspi/COOL_Spiru/update' 		// TamataFarm Topic 
var mqttServer = ""; // '10.3.141.1';
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
		topicTamata = obj.system.mqttTopic + '/update';
		mqttServer = obj.system.mqttServer;
		user = obj.system.user;
		id = obj.system.id;
		
		getCoolCoSensors();
});

function getCoolCoSensors(){
	var clientRasPi = mqtt.connect({host:mqttServer,port:1883})
	/* JSON */
	clientRasPi.on('connect',function(){
		clientRasPi.subscribe(topicTamata)
		logger.info('Connected to Topic TamataSpiru')
		
	})
	
	clientRasPi.on('message', function(topic, message) {
		logger.info('Open topic = %s',topic)		
		//JSON Analyse 
		var jsonCool = JSON.parse(message);
		objJSON = {state:
				{reported:
					{
						User : user, 
						Id : id,
						timestamp : moment
					}
				}
		}

		//_.set(objJSON, 'state.reported.lat',"49°33'1029N")
		//_.set(objJSON, 'state.reported.lon',"01°51'3173W")
		_.set(objJSON, 'state.reported.Temp1',parseFloat(jsonCool.state.reported.Temp1))
		_.set(objJSON, 'state.reported.Temp',parseFloat(jsonCool.state.reported.Temp))
		_.set(objJSON, 'state.reported.Humi',parseFloat(jsonCool.state.reported.Humi))
		_.set(objJSON, 'state.reported.Pres',parseFloat(jsonCool.state.reported.Pres))
		_.set(objJSON, 'state.reported.Vis',parseFloat(jsonCool.state.reported.Vis))
		_.set(objJSON, 'state.reported.IR',parseFloat(jsonCool.state.reported.IR))
		_.set(objJSON, 'state.reported.UV',parseFloat(jsonCool.state.reported.UV))
		_.set(objJSON, 'state.reported.Moist',parseFloat(jsonCool.state.reported.Moist))
		_.set(objJSON, 'state.reported.Bat',parseFloat(jsonCool.state.reported.Bat))
		_.set(objJSON, 'state.reported.Sig',parseFloat(jsonCool.state.reported.Sig))
		_.set(objJSON, 'state.reported.HeatTARGET',parseFloat(jsonCool.state.reported.HeatTARGET))
		_.set(objJSON, 'state.reported.r',parseFloat(jsonCool.state.reported.r))
		_.set(objJSON, 'state.reported.g',parseFloat(jsonCool.state.reported.g))
		_.set(objJSON, 'state.reported.b',parseFloat(jsonCool.state.reported.b))
		_.set(objJSON, 'state.reported.lux',parseFloat(jsonCool.state.reported.lux))
		_.set(objJSON, 'state.reported.colorTemp',parseFloat(jsonCool.state.reported.colorTemp))
		_.set(objJSON, 'state.reported.colorHex',jsonCool.state.reported.colorHex)
		_.set(objJSON, 'state.reported.onHeat',parseFloat(jsonCool.state.reported.onHeat))
		_.set(objJSON, 'state.reported.onLight',parseFloat(jsonCool.state.reported.onLight))
		_.set(objJSON, 'state.reported.onBubler',parseFloat(jsonCool.state.reported.onBubler))
		
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
			//var client = mqtt.connect('mqtt://search-mosquitto-k34joo6mwdg7ynzkts6epsxuqy.eu-west-1.es.amazonaws.com')
			//logger.info('Temp ='+temperature+' R='+red+'/G='+green+'/B='+blue)
			logger.info('Finale : Mqtt transfert...')
			var client = mqtt.connect({host:'52.17.46.139', port:1883})
			
			client.on('connect',function() {
				client.publish('$aws/things/tamatataRASPI/shadow/update',JSON.stringify(objJSON))
				logger.info("Publish on Kibana "+moment)
				client.end()
			})
			
			// Write Temp File waiting for connection available.
			//jsonfile.writeFile(file,objJSON, function(err) { if (err) throw err});
			msgCount = 0;
			
		}
}



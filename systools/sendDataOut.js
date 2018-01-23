/* Script lauchned automatically to get Data from Broker to send them  to DataBase*/
/* ------------------------------------------------------------------------------ */
var dns = require('dns')
let fs = require('fs')
var jsonfile = require('jsonfile')
var moment = require('moment')
var mqtt = require('mqtt')
var winston = require('winston');				//Logger
require('winston-logrotate');
var jsonexport = require('jsonexport');

// Load the full build.
var _ = require('lodash');

/* I2C Necessary from Raspberry connected with I2C and Arduino
var i2c = require('i2c')
var address = 0x04;
var wire = new i2c(address, {device: '/dev/i2c-1'});*/

/* MQTT
topic : $aws/things/tamatataRASPI/shadow/update/delta */

/* Config JSON indent mode */
configFile = "../config/config.json";
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

var file = '../log/ExportTamataFarm_'+moment+'.json'
var msgCount = 0

//---------------------
//get CoolCoSpiru Board
//---------------------
jsonfile.readFile(configFile, function(err, obj) {
		logger.info("readFile : "+JSON.stringify(obj))
		if (err) throw err;
		mqttTopic = obj.system.mqttTopic + '/update';
		//mqttTopic = 'dev/update';
		mqttServer = obj.system.mqttServer;
		// mqttAWS = obj.system.mqttAWS;
		mqttAWS = 'tamataraspi.local';
		user = obj.system.user;
		getCoolCoSensors();
});

function webConnected() {
	dns.resolve('www.Google.fr', function(err){
		if (err) {
			logger.info("Internet connection status : No connection !!! ");
			return false;
		} else {
			logger.info('Internet connection status : connected.');
			return true;
		}
	}); 
}

/* Function to Send JSON to MQTT Broker
OR on Local file to save if Internet is not available
*/ 	
function finale()
{
	var dataTempFile = "../log/dataTempFile.json"
	logger.info('Finale : Mqtt transfert...')

	dns.resolve('www.Google.fr', function(err){
		if (err) {
			logger.info('Finale not Connected...') 
			saveDataLocal()
		} else {
			logger.info('Finale Connected...') 
			var oldData = {}
			/* Checking for TempLog file */
			jsonfile.readFile(dataTempFile, function(err, data) {
				if (err) return console.log(err);
				var objJSON;

				for (var rowData in data.state ) {
					objJSON = {}
					console.log('rowData = ' + rowData );
					//console.log( JSON.stringify(data.state[rowData]) );

					for(var exKey in data.state[rowData].reported ) {
			    		// console.log("key:"+exKey+", value:"+data.state[rowData].reported[exKey]);
			    		_.set(objJSON, "state.reported."+exKey, data.state[rowData].reported[exKey] )
					}

					console.log('objJSON = ' + JSON.stringify(objJSON))
					sendDataToBroker(objJSON);
				}

				
			});
			
		}
	}); 
}

/* function to saev objJSON onto Local File */
/* **************************************** */ 
function saveDataLocal(){
	logger.info("saveDataLocal")
	var dataTempFile = "../log/datastorage.json";
	// Write Temp File waiting for connection available.
	jsonfile.writeFileSync(dataTempFile,objJSON, {flag: 'a'}, function(err) { if (err) throw err});
}

/* Function to Send obJJSON to Broker */
/* ********************************** */
function sendDataToBroker(objJSON){
	logger.info("sendDataToBroker")
	var client = mqtt.connect({host:mqttAWS, port:1883})
	client.on('connect',function() {	
		// client.publish(mqttTopic,JSON.stringify(objJSON))
		logger.info('sendDataToBroker ... ');
		client.publish('dev/update',JSON.stringify(objJSON))
		logger.info("Publish on "+mqttTopic+" at "+moment)
		client.end()
	})
}


/* Function to get message from Mqtt */
/* recording Json format onto shared object "objJSON"
/* ********************************* */
function getCoolCoSensors(){
	logger.info("getCoolCoSensors")
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
		finale();
	})
}




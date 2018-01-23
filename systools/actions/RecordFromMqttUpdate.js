var moment = require('moment')
moment = moment().format('YYYY-MM-DDTHH:mm:ss\\Z')

var mqtt = require('mqtt')
var jsonfile = require('jsonfile')
var mqttTopic, mqttServer;
var _ = require('lodash');

var configFile = "../../config/config.json";
var dataStorage = "../../log/datastorage.json";
var client;

jsonfile.spaces = 4;

jsonfile.readFile(configFile, function(err, obj) {
	if (err) throw err;

	mqttTopic = obj.system.mqttTopic + '/update';
	console.log("Result parse config.json : " + mqttTopic);
	mqttServer = obj.system.mqttServer;
	console.log("Result parse config.json : " + mqttServer);
	client = mqtt.connect('mqtt://10.3.141.1');

	client.on('connect', function () {
	console.log("mqttTopic : " + mqttTopic);
	    client.subscribe(mqttTopic)
	    console.log("connect to topic ")
	    //jsonfile.writeFile(dataStorage,"Beginning Tracking --- "+ moment, function(err) { if (err) throw err});
	})

	client.on('message', function (topic, message) {
		var obj = ""
		console.log('MESSAGE = ' + message);
		writeFile(JSON.parse(message));	
	}) 
})

function writeFile(objJSON) {
		console.log('Writing file.... ');
		jsonfile.writeFile(dataStorage,JSON.stringify([{objJSON}]),{flag: 'a'}, function(err) { if (err) throw err});
		console.log('file saved...');	
	}
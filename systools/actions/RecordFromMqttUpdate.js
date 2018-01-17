var moment = require('moment')
moment = moment().format('YYYY-MM-DDTHH:mm:ss\\Z')

var mqtt = require('mqtt')
var jsonfile = require('jsonfile')
var mqttTopic, mqttServer;
var _ = require('lodash');

var configFile = "/home/bibi/node/config/config.json";
// dataStorage = "/home/bibi/node/log/datastorage.json";
var dataStorage = "datastorage.json";

jsonfile.spaces = 4;
jsonfile.readFile(configFile, function(err, obj) {
		if (err) throw err;
		mqttTopic = obj.system.mqttTopic + '/update';
		mqttServer = obj.system.mqttServer;
})

var datastorage_SAV = {};

var client  = mqtt.connect('mqtt://10.3.141.1')
client.on('connect', function () {
    client.subscribe(mqttTopic)
    console.log("connect to topic ")
    jsonfile.writeFile(dataStorage,"Beginning Tracking --- "+ moment, function(err) { if (err) throw err});
})

var objJSON = {};

client.on('message', function (topic, message) {
	var jsonCool = JSON.parse(message.toString());
	//JSON Analyse 
	for(var exKey in jsonCool.state.reported) {
		_.set(objJSON, "state.reported."+exKey, jsonCool.state.reported[exKey])
	}

	writeFile(objJSON);
})

function writeFile(objJSON) {
	console.log('Writing file.... ');
	jsonfile.writeFile(dataStorage,objJSON["state"],{flag: 'a'}, function(err) { if (err) throw err});
	console.log('file saved...');	
}


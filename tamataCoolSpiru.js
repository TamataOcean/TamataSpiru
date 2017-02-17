var i2c = require('i2c')
let fs = require('fs')
var jsonfile = require('jsonfile')
var moment = require('moment')
var mqtt = require('mqtt')

// Load the full build.
var _ = require('lodash');

/* I2C */
var address = 0x04;
var wire = new i2c(address, {device: '/dev/i2c-1'});

/* MQTT
topic : $aws/things/tamatataRASPI/shadow/update/delta */
var topicTamata = 'tamataraspi/spiru'

/* Variables */
var temperature;
var red;
var green;
var blue;

/* Check Time */
moment = moment().format('YYYY-MM-DDTHH:mm:ss\\Z')
console.log('TamataSpiru Transfert ('+moment+')')

/* JSON */
var objJSON = {state:
				{reported:
					{
						User : "TamataSpiru", 
						Id : "TamataRasPi",
						timestamp : moment
					}
				}
	}

var file = '/tmp/ExportSpiru_'+moment+'.json'
var msgCount = 0
	
wire.on('data', function(data) {
	//console.log(data)
  // result for continuous stream contains data buffer, address, length, timestamp
});
	
//get Temperature from Arduino
wire.writeByte(0x02, function(err){}); 
wire.readBytes(0x01, 6, function(err, res) 
{
	temperature = parseFloat(res.toString('ascii'),10);
	objJSON.state.reported.Temp1 = temperature;
	//console.log('temp :'+temperature);
	msgCount++;
	finale();
});

//get RED color from Arduino
wire.writeByte(0x03, function(err){}); 
wire.readBytes(0x01, 6, function(err, res) 
{
	red = parseInt(res.toString('ascii'),10);
	//console.log('red: '+red);
	objJSON.state.reported.R = red;
	msgCount++;
	finale();
});

//get GREEN color from Arduino
wire.writeByte(0x04, function(err) {});
wire.readBytes(0x01, 6, function(err, res) 
{
	green = parseInt(res.toString('ascii'));
	//console.log('green: '+green)
	objJSON.state.reported.G = green;
	msgCount++;
	finale();
});

//get BLUE color from Arduino
wire.writeByte(0x05, function(err) {});
wire.readBytes(0x01, 6, function(err, res) 
{
	blue = parseInt(res.toString('ascii'));
	//console.log('bleu :'+blue)
	objJSON.state.reported.B = blue;
	msgCount++;
	finale();
});

//get CoolCo Board Sensors
getCoolCoSensors();
function getCoolCoSensors(){
	var clientRasPi = mqtt.connect({host:'tamataraspi8go.local',port:1883})
	/* JSON */
	clientRasPi.on('connect',function(){
		clientRasPi.subscribe('tamataraspi/spiru')
		console.log('Connected to Topic TamataSpiru')
	})
	
	clientRasPi.on('message', function(topic, message) {
		//console.log('get Message from TamataSpiru : %s', message)
		console.log('Open topic = %s',topic)		
		//JSON Analyse 
		var jsonCool = JSON.parse(message);
		_.set(objJSON, 'state.reported.Temp',parseFloat(jsonCool.state.reported.Temp))
		_.set(objJSON, 'state.reported.Humi',parseFloat(jsonCool.state.reported.Humi))
		_.set(objJSON, 'state.reported.Pres',parseFloat(jsonCool.state.reported.Pres))
		_.set(objJSON, 'state.reported.Vis',parseFloat(jsonCool.state.reported.Vis))
		_.set(objJSON, 'state.reported.IR',parseFloat(jsonCool.state.reported.IR))
		_.set(objJSON, 'state.reported.UV',parseFloat(jsonCool.state.reported.UV))
		_.set(objJSON, 'state.reported.Moist',parseFloat(jsonCool.state.reported.Moist))
		_.set(objJSON, 'state.reported.Bat',parseFloat(jsonCool.state.reported.Bat))
		_.set(objJSON, 'state.reported.Sig',parseFloat(jsonCool.state.reported.Sig))
		
		console.log('JSON stringify : '+JSON.stringify(objJSON))
		clientRasPi.end()
		msgCount++;
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
			//console.log('Temp ='+temperature+' R='+red+'/G='+green+'/B='+blue)
			console.log('Finale : Mqtt transfert...')
			var client = mqtt.connect({host:'52.17.46.139', port:1883})
			
			client.on('connect',function() {
				client.publish('$aws/things/tamatataRASPI/shadow/update',JSON.stringify(objJSON))
				console.log("Publish on Kibana "+moment)
				//console.log("objJSON="+JSON.stringify(objJSON))
				client.end()
			})
			
			// Write Temp File waiting for connection available.
			jsonfile.writeFile(file,objJSON, function(err) { if (err) throw err});
			msgCount = 0;
			
		}
}


//Déclaration Server Web
/*
var http = require('http');
var serverW = http.createServer()
serverW.on('request',function (request, response){
	fs.readFile('index.html', function (err,data){
		if (err) throw err
		response.writeHead(200, {'Content-type': 'text/html; charset=utf-8'})
		response.write(data)
		response.write('<p>Temperature ='+temperature+'</p>')
		response.write('<p>Color :</p>')
		response.write('Red = '+red+'</BR>')
		response.write('Green = '+green+'</BR>')
		response.write('Blue = '+blue+'</BR>')
	})	
});
//Launching WebServer
//serverW.listen(8080)
*/



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
var topicTamata = 'tamataraspi/spiru/update'

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
						Id : "TamataFarm",
						timestamp : moment
					}
				}
	}

var file = '/tmp/ExportTamataFarm_'+moment+'.json'
var msgCount = 0

//---------------------
//get CoolCoSpiru Board
//---------------------
getCoolCoSensors();
function getCoolCoSensors(){
	var clientRasPi = mqtt.connect({host:'tamataraspi8go.local',port:1883})
	/* JSON */
	clientRasPi.on('connect',function(){
		clientRasPi.subscribe('tamataraspi/spiru/update')
		console.log('Connected to Topic TamataSpiru')
	})
	
	clientRasPi.on('message', function(topic, message) {
		//console.log('get Message from TamataSpiru : %s', message)
		console.log('Open topic = %s',topic)		
		//JSON Analyse 
		var jsonCool = JSON.parse(message);
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
		_.set(objJSON, 'state.reported.onHeat',parseFloat(jsonCool.state.reported.onHeat))
		_.set(objJSON, 'state.reported.onLight',parseFloat(jsonCool.state.reported.onLight))
		_.set(objJSON, 'state.reported.onBubler',parseFloat(jsonCool.state.reported.onBubler))
		
		console.log('JSON stringify : '+JSON.stringify(objJSON))
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





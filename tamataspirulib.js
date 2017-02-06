tamataspiru = module.exports = 
{
	/* ------------------------------------- */
	/* ------------- VARIABLES ------------- */
	/* ------------------------------------- */
	fs : require('fs'),
	jsonfile : require('jsonfile'),
	_ : require('lodash'),
	/* ---- I2C ----- */
	i2c : require('i2c'),
	i2c_address : 0x04,
	i2c_wire : null,
	/* ---- MQTT ---- */
	mqtt : require('mqtt'),
	topicTamata : 'tamataraspi/spiru',
	
	/* ---- Sensors Value ---- */
	temperature : 0,
	red : 0,
	green : 0,
	blue : 0,
	objJSON : {state:
				{reported:
					{
						user : "TamataSpiru", 
						id : "TamataRasPi",
						T : require('moment')().format('YYYY-MM-DDTHH:mm:ss\\Z'),
						Temp : 0
					}
				}
	},
	msgCount : 0,
	
	/* ------------------------------------- */
	/* ------------- FUNCTIONS ------------- */
	/* ------------------------------------- */
	finale : function(){
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
	},
	
	getTemperature : function() {
		this.i2c_wire = new this.i2c(this.i2c_address, {device: '/dev/i2c-1'});
		this.i2c_wire.on('data', function(data) {
		//console.log(data)
		// result for continuous stream contains data buffer, address, length, timestamp
		});
			
		//get Temperature
		this.i2c_wire.writeByte(0x02, function(err){}); 
		this.i2c_wire.readBytes(0x01, 6, function(err, res) 
		{
			this.temperature = parseFloat(res.toString('ascii'),10);
			//this.objJSON.state.reported.Temp = temperature;
			console.log('getTemperature (Temp = '+temperature+')');
			//this.msgCount++;
			//this.finale();
			return temperature;
		});
	},
	
	getCoolCoSensors : function getCoolCoSensors(){
		var clientRasPi = this.mqtt.connect({host:'tamataraspi.local',port:1883})
		/* JSON */
		clientRasPi.on('connect',function(){
			clientRasPi.subscribe('tamataraspi/spiru')
			console.log('getCoolCoSensorsFunction : Connected to Topic TamataSpiru')
		});
		
		clientRasPi.on('message', function(topic, message) {
			//console.log('get Message from TamataSpiru : %s', message)
			console.log('getCoolCoSensorsFunction : Open topic = %s / %s',topic,message)		
			//JSON Analyse 
			var jsonCool = JSON.parse(message);
			/*
			require('lodash').set(this.objJSON, 'state.reported.2T',parseFloat(jsonCool.state.reported.Temp))
			require('lodash').set(this.objJSON, 'state.reported.2H',parseFloat(jsonCool.state.reported.Humi))
			require('lodash').set(this.objJSON, 'state.reported.2P',parseFloat(jsonCool.state.reported.Pres))
			require('lodash').set(this.objJSON, 'state.reported.3V',parseFloat(jsonCool.state.reported.Vis))
			require('lodash').set(this.objJSON, 'state.reported.3I',parseFloat(jsonCool.state.reported.IR))
			require('lodash').set(this.objJSON, 'state.reported.3U',parseFloat(jsonCool.state.reported.UV))
			require('lodash').set(this.objJSON, 'state.reported.4M',parseFloat(jsonCool.state.reported.Moist))
			require('lodash').set(this.objJSON, 'state.reported.Bat',parseFloat(jsonCool.state.reported.Bat))
			require('lodash').set(this.objJSON, 'state.reported.Sig',parseFloat(jsonCool.state.reported.Sig))
			
			console.log('JSON stringify : '+JSON.stringify(jsonCool))
			*/
			//console.log('JSON stringify : '+JSON.stringify(this.objJSON))
			clientRasPi.end()
			
			//msgCount++;
			//finale();
			console.log('getCoolCoSensorsFunction End');
			return jsonCool;
		})
	}
};
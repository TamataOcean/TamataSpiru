
const DEBUG = true;

const Influx = require('influx')
const FieldType = Influx.FieldType;

const mongoose = require ('mongoose')
var mongodbURI = 'mongodb://localhost:27017/dataspiru'; 
const Sensor = require ('./sensorSchema')
console.log('test...');

const influx = new Influx.InfluxDB({
  database: 'dataspiru',
  host: 'localhost',
  port: 8086,
  username: 'test',
  password: 'test',
  schema: [
    {
      measurement: 'sensors',
      fields: {
      	user : 			FieldType.STRING,
        timestamp : 	FieldType.timestamp,
        mac: 			FieldType.STRING,
        visibleLight: 	FieldType.FLOAT,
        infraRed:   	FieldType.FLOAT,
        ultraViolet: 	FieldType.FLOAT,
        Temperature: 	FieldType.FLOAT,
        Pressure:    	FieldType.FLOAT,
        Humidity:    	FieldType.FLOAT,
        Vbat:        	FieldType.FLOAT,
        soilMoisture: 	FieldType.FLOAT,
        TempMC:     	FieldType.FLOAT,
        RGBSensor_r: 	FieldType.FLOAT,
        RGBSensor_g: 	FieldType.FLOAT,
        RGBSensor_b : 	FieldType.FLOAT,
        RGBSensor_colorTemp: FieldType.FLOAT
      },
      tags: [ 'sensor' ]
    }
  ]
});

influx.getDatabaseNames()
  .then(names => {
    if ( !names.includes('dataspiru') ) {
      console.log('First connection... create database dataspiru');	
      return influx.createDatabase('dataspiru')
    }
  })
  .then(() => {
    //http.createServer(app).listen(3000, function () {
	console.log('Connected to InFlux');
	mongoose.connect(mongodbURI)
	
	.then(() => {
	      if (DEBUG) console.log('connection to mongoose Ok !');
	      Sensor.find( {} , function (err, sensors){ 
	      //Sensor.find( {remoteSaved:null} , function (err, sensors){ 
	      //Sensor.find( { _id : "5a686c11ad49c7736eb55bef" } , function (err, sensors){ 
	    	if (err) console.log(err);

	    	console.log('writePoints to Influx...');
	    	for (var key in sensors ) {
	    		// jsonRecord = sensors[key].state.reported.toJSON()
	    		jsonRecord = sensors[key].state.reported.toJSON()
	    		if (DEBUG) console.log('jsonRecord = '+ JSON.stringify(jsonRecord) );
	    		
	    		influx.writePoints([
					{
					measurement: 'response_times',
					tags: { sensor: "CoolBoardSensors" },
					fields: { 
						user : 				jsonRecord.user,
						timestamp : 		Date.parse(jsonRecord.timestamp),
					 	mac: 				jsonRecord.mac,
						visibleLight: 		jsonRecord.visibleLight,
						infraRed:   		jsonRecord.infraRed,
						ultraViolet: 		jsonRecord.ultraViolet,
						Temperature: 		jsonRecord.Temperature,
						Pressure:   		jsonRecord.Pressure,
						Humidity:   		jsonRecord.Humidity,
						Vbat:       		jsonRecord.Vbat,
						soilMoisture: 		jsonRecord.soilMoisture,
						TempMC:     		jsonRecord.TempMC,
						RGBSensor_r: 		jsonRecord.RGBSensor_r,
						RGBSensor_g: 		jsonRecord.RGBSensor_g,
						RGBSensor_b: 		jsonRecord.RGBSensor_b,
						RGBSensor_colorTemp: jsonRecord.RGBSensor_colorTemp
						}	
					}
				]).catch(err => {
					console.error(`Error saving data to InfluxDB! ${err.stack}`)
				}).then( () => {
					console.log('visibleLight = ' + jsonRecord.visibleLight );
					console.log('Doc ID=' + sensors[key]._id + " pushed to InFlux" );
				})

				
	    		/*
	    		influx.writePoints([
					measurement: 'sensors',
					fields: {
						user : 				sensors[key].user,
					},
					tags: [ 'sensor' ]
	    		])
				.catch(err => {
					console.error(`Error saving data to InfluxDB! ${err.stack}`)
				})
				.then(() => {
					console.log('Doc ID=' + sensors[key]._id + " pushed to InFlux" );
				})*/

	    	}
		}) 
 		.then(() => {
       		if (DEBUG) console.log('end Find... ');
       		mongoose.connection.close();
    	})
	    .catch( err => {
	    	console.log('Error on Find = ' + err );})
	    })
	            
	//             if (DEBUG) console.log('found some records... ');
	// 			{
	//        }).then(
 //            () => {
 //               if (DEBUG) console.log('end Find... ');
 //               mongoose.connection.close();
 //            },
 //            err => {console.log('Error on Find = ' + err );})
	//     })
  })
  .catch(err => {
    console.error(`Error creating Influx database!`)
  });

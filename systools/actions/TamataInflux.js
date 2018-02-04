/* Save InfluxDB */
var DEBUG = true;

const Influx = require('influx')
const FieldType = Influx.FieldType;

function TamataInfluxDB ( jsonObject, measurement ) {
	this.measurement = measurement;
	this.config = jsonObject;

	console.log('InfluxDB constructor...');
	console.log('Config = ' + JSON.stringify ( jsonObject ) );
	console.log('InfluxDB Measurement = ' + measurement);
	this.connect()
}

TamataInfluxDB.prototype.connect = function() {
	console.log("InfluxDB - connect function");
	if ( this.measurement === "sensor") {
		console.log('this a sensor to save to InfluxDB');
		this.influx = new Influx.InfluxDB({
	        database: this.config.database,
	        host: this.config.host,
	        port: this.config.port,
	        username: this.config.username,
	        password: this.config.password,
	        schema: [
	          {
	            measurement: this.measurement,
	            fields: {
	              user :         FieldType.STRING,
	              timestamp :  FieldType.FLOAT,
	              mac:         FieldType.STRING,
	              visibleLight:   FieldType.FLOAT,
	              infraRed:    FieldType.FLOAT,
	              ultraViolet:    FieldType.FLOAT,
	              Temperature:    FieldType.FLOAT,
	              Pressure:       FieldType.FLOAT,
	              Humidity:       FieldType.FLOAT,
	              Vbat:           FieldType.FLOAT,
	              soilMoisture:   FieldType.FLOAT,
	              TempMC:      FieldType.FLOAT,
	              RGBSensor_r:    FieldType.FLOAT,
	              RGBSensor_g:    FieldType.FLOAT,
	              RGBSensor_b :   FieldType.FLOAT,
	              RGBSensor_colorTemp: FieldType.FLOAT
	            },
	            tags: [ 'sensor' ]
	          }
	        ]
	    });
	}
	else if (this.measurement === "jetpack") {
		console.log('this a jetpack to save to InfluxDB');
		this.influx = new Influx.InfluxDB({
	        database: 'dataspiru',
	        host: 'localhost',
	        port: 8086,
	        username: 'test',
	        password: 'test',
	        schema: [
	          {
	            measurement: this.measurement,
	            fields: {
	               Act0_Value: FieldType.FLOAT,
	               Act1_Value: FieldType.FLOAT,
	               Act2_Value: FieldType.FLOAT,
	               Act3_Value: FieldType.FLOAT,
	               Act4_Value: FieldType.FLOAT,
	               Act5_Value: FieldType.FLOAT,
	               Act6_Value: FieldType.FLOAT,
	               Act7_Value: FieldType.FLOAT
	            },
	            tags: [ 'jetpack' ]
	          }
	        ]
      	});
	}
};

TamataInfluxDB.prototype.save = function( jsonRecord, measurement ) {
	if (DEBUG) console.log('InfluxDB save function...');
   this.influx.getDatabaseNames()
   .then(names => {
    if ( !names.includes('dataspiru') ) {
      if (DEBUG) console.log('First connection... create database dataspiru');  
      return this.influx.createDatabase('dataspiru')
    }
   })
   .then( () => {
      if (DEBUG) console.log('database : dataspiru found');
      if (DEBUG) console.log('jsonRecord = '+ JSON.stringify(jsonRecord) );

      if (measurement ==="sensor" ){
         this.influx.writePoints([
               {
               measurement: measurement,
               tags: { sensor: "CoolBoardSensors" },
               fields: { 
                  user :            jsonRecord.state.reported.user,
                  timestamp :       Date.parse(jsonRecord.state.reported.timestamp),
                  mac:              jsonRecord.state.reported.mac,
                  visibleLight:     jsonRecord.state.reported.visibleLight,
                  infraRed:         jsonRecord.state.reported.infraRed,
                  ultraViolet:      jsonRecord.state.reported.ultraViolet,
                  Temperature:      jsonRecord.state.reported.Temperature,
                  Pressure:         jsonRecord.state.reported.Pressure,
                  Humidity:         jsonRecord.state.reported.Humidity,
                  Vbat:             jsonRecord.state.reported.Vbat,
                  soilMoisture:     jsonRecord.state.reported.soilMoisture,
                  TempMC:           jsonRecord.state.reported.TempMC,
                  RGBSensor_r:      jsonRecord.state.reported.RGBSensor_r,
                  RGBSensor_g:      jsonRecord.state.reported.RGBSensor_g,
                  RGBSensor_b:      jsonRecord.state.reported.RGBSensor_b,
                  RGBSensor_colorTemp: jsonRecord.state.reported.RGBSensor_colorTemp
                  }  
               }]).catch(err => {
                  console.error(`Error saving Sensor data to InfluxDB! ${err.stack}`);
                  return;
               }).then( () => {
                  console.log('Doc type '+measurement +' pushed to InFlux'  );
                  console.log('\n');
               });
      } 
      else if (measurement ==="jetpack" ){
         //Inverted case to considere...
         var ActInverted = !(jsonRecord.state.reported.Act7)

         console.log('Act7 = ' + jsonRecord.state.reported.Act7 );
         console.log('ActInverted = ' + ActInverted );

         this.influx.writePoints([
               {
               measurement: measurement,
               tags: { jetpack: "CoolBoardJetpack" },
               fields: { 
                  Act0_Value: convertBoolean(jsonRecord.state.reported.Act0),
                  Act1_Value: convertBoolean(jsonRecord.state.reported.Act1),
                  Act2_Value: convertBoolean(jsonRecord.state.reported.Act2),
                  Act3_Value: convertBoolean(jsonRecord.state.reported.Act3),
                  Act4_Value: convertBoolean(jsonRecord.state.reported.Act4),
                  Act5_Value: convertBoolean(jsonRecord.state.reported.Act5),
                  Act6_Value: convertBoolean(jsonRecord.state.reported.Act6),
                  Act6_Value: convertBoolean(jsonRecord.state.reported.Act6),
                  Act7_Value: convertBoolean(jsonRecord.state.reported.Act7)
                  }  
               }]).catch(err => {
                  console.error(`Error saving Jetpack data to InfluxDB! ${err.stack}`);
                  return;
               }).then( () => {
                  console.log('Doc type '+measurement +' pushed to InFlux' );
                  console.log('\n');
               });
      } else 
      {
         console.log('Mqtt message Type not managed... yet ;-) !!! ');
      }


   })
   .catch(err => {
       console.error(`Error creating Influx database!`)
       console.log(`${err.stack}`);
       return;
   });
	// body...
};


TamataInfluxDB.prototype.convertBoolean = function ( boolean ) {
   if (DEBUG) console.log('convertBoolean entry '+  boolean );
   var result = 0;
   if (boolean) result = 1;
   if (DEBUG) console.log('converted to '+ result );
   return result;
}

module.exports = TamataInfluxDB;

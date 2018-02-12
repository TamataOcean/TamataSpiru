/* Save MongoDB */
var DEBUG = true;

var mongoose = require('mongoose');
var Sensor = require('../schema/sensorSchema')
var JetPack = require('../schema/jetPackSchema')

/* Inpire from 
https://github.com/Automattic/mongoose/blob/master/examples/schema/storing-schemas-as-json/index.js

// modules
var mongoose = require('../../../lib');
var Schema = mongoose.Schema;

// parse json
var raw = require('./schema.json');

// create a schema
var timeSignatureSchema = Schema(raw);

// compile the model
var TimeSignature = mongoose.model('TimeSignatures', timeSignatureSchema);

// create a TimeSignature document
var threeFour = new TimeSignature({
  count: 3,
  unit: 4,
  description: '3/4',
  additive: false,
  created: new Date,
  links: ['http://en.wikipedia.org/wiki/Time_signature'],
  user_id: '518d31a0ef32bbfa853a9814'
});

// print its description
console.log(threeFour);
*/

class TamataMongoDB {
   constructor ( jsonObject, measurement ) {
   	this.measurement = measurement;
   	this.config = jsonObject;
      this.sensor = "";
      this.jetpack = "";
   	if (DEBUG) {
         console.log('MongoDB constructor...');
         console.log('Config = ' + JSON.stringify ( this.config ) );
         console.log('MongoDB Measurement = ' + this.measurement);
         console.log('MongoDB URI = ' + this.config.URI);
      }
   }

   /*
   connect() {
   	console.log("MongoDB - connect function");
      return new Promise((resolve, reject) => {
      });
   }
   */

   save( jsonRecord, measurement ) {
      if (DEBUG) console.log('MongoDB - save function...');

      mongoose.connect(this.config.URI, function (err, db) {
         if(err) throw console.log(err);
         console.log('connected to db, begin... ');
      })
      .then( () => {
         if ( measurement === "sensor") {
            console.log('this a sensor to save to MongoDB');
            this.sensor = new Sensor ({
               state:{
                  reported:{
                     user :      jsonRecord.state.reported.user,
                     timestamp : jsonRecord.state.reported.timestamp,
                     mac:        jsonRecord.state.reported.mac,
                     visibleLight:jsonRecord.state.reported.visibleLight,
                     infraRed:   jsonRecord.state.reported.infraRed,
                     ultraViolet:jsonRecord.state.reported.ultraViolet,
                     Temperature:jsonRecord.state.reported.Temperature,
                     Pressure:   jsonRecord.state.reported.Pressure,
                     Humidity:   jsonRecord.state.reported.Humidity,
                     Vbat:       jsonRecord.state.reported.Vbat,
                     soilMoisture:jsonRecord.state.reported.soilMoisture,
                     TempMC:     jsonRecord.state.reported.TempMC,
                     RGBSensor_r:jsonRecord.state.reported.RGBSensor_r,
                     RGBSensor_g:jsonRecord.state.reported.RGBSensor_g,
                     RGBSensor_b:jsonRecord.state.reported.RGBSensor_b,
                     RGBSensor_colorTemp: jsonRecord.state.reported.RGBSensor_colorTemp
                  } 
               }
            });

            this.saveSensor( jsonRecord )
            
            // .then( () => {
            // });
         }
         else if ( measurement === "jetpack") {
            console.log('this a jetpack to save to MongoDB');
            this.saveJetPack ( jsonRecord )
         }
      })
   }

   saveSensor(jsonRecord ) {    
      if (DEBUG) console.log('MongoDB saveSensor function...');
     
      console.log('-------------------- iminent save to MongoDB');
      this.sensor.save(function(err){
         if (err) console.log(err);
         // if (DEBUG) console.log('sensor parsed = ', this.sensor.toJSON() );
         console.log(this.sensor._id +  ' saved Local & Remote');
      })
   }

   saveJetPack(jsonRecord ) {
      if (DEBUG) console.log('MongoDB saveJetPack function...');

   }
}

module.exports = TamataMongoDB;

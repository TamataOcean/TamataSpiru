/*--------------------------------------------
This script is used :
- to listen on Mqtt Update topic 
- and save message onto mongoDB
---------------------------------------------*/
var DEBUG = false;

var mqtt = require('mqtt'); //includes mqtt server 
var moment = require('moment')
var Sensor = require('./sensorSchema')
var mongoose = require('mongoose');
var mongodbURI = 'mongodb://localhost:27017/dataspiru'; 
var deviceRoot = "$aws/things/6001941D9FAA/shadow/update"; 
var db;

// Prod to AWS
// var mqttServer = "52.17.46.139";
// var mqttTopic = "$aws/things/6001941D9FAA/shadow/update";

// Dev env
var mqttServer = "10.3.141.1";
var mqttTopic = "dev/update";

mongoose.connect(mongodbURI, function (err, db) {
   if(err) throw console.log(err);
   console.log('connected to db, begin... ');   
   
   //Listening on Mqtt Broker
   client = mqtt.connect('mqtt://10.3.141.1');
   client.subscribe("$aws/things/6001941D9FAA/shadow/update"); 
   client.on('message', insertEvent);                    
});

// Function that record the data in the MongoDataBase 
// Check && Internet available ? send to Mqtt AWS 
function insertEvent(topic,message) {

   var parsedMessage = JSON.parse(message);
   
   if (DEBUG) console.log('Insert Message : ' + JSON.stringify(parsedMessage) ) ;
   if (DEBUG) console.log('-----------------------------');
   
   var sensor = new Sensor ({
      state:{
         reported:{
            user :      parsedMessage.state.reported.user,
            timestamp : parsedMessage.state.reported.timestamp,
            mac:        parsedMessage.state.reported.mac,
            visibleLight:parsedMessage.state.reported.visibleLight,
            infraRed:   parsedMessage.state.reported.infraRed,
            ultraViolet:parsedMessage.state.reported.ultraViolet,
            Temperature:parsedMessage.state.reported.Temperature,
            Pressure:   parsedMessage.state.reported.Pressure,
            Humidity:   parsedMessage.state.reported.Humidity,
            Vbat:       parsedMessage.state.reported.Vbat,
            soilMoisture:parsedMessage.state.reported.soilMoisture,
            TempMC:     parsedMessage.state.reported.TempMC,
            RGBSensor_r:parsedMessage.state.reported.RGBSensor_r,
            RGBSensor_g:parsedMessage.state.reported.RGBSensor_g,
            RGBSensor_b:parsedMessage.state.reported.RGBSensor_b,
            RGBSensor_colorTemp: parsedMessage.state.reported.RGBSensor_colorTemp
         } 
      }
   });

   /* Checking Internet Connection 
   different way tested, best with exec ping function.
   require('dns').lookupService('8.8.8.8', 53, function(err, hostname, service){   
   require('dns').lookup('google.com',function(err) {
   require('dns').resolve('www.Google.fr', function(err){
   */
   var exec = require('child_process').exec, child;
   child = exec('ping -c 1 google.com', function(error, stdout, stderr){
   
      if (error === null) {
         if (DEBUG) console.log('... Internet connected');
         /* Sending to Mqtt */
         client = mqtt.connect( "mqtt://" + mqttServer );
         client.on('connect',function(err) {
            client.publish(mqttTopic, JSON.stringify(parsedMessage) )
            sensor.remoteSaved = moment.now()
            sensor.save(function(err){
               if (err) console.log(err);
               if (DEBUG) console.log('sensor parsed = ', sensor);
               console.log(sensor._id +' saved Local & Remote');
               syncInflux(sensor);
            })
         })
      } else {
         if (DEBUG) console.log('... Internet no connection !');
         sensor.save(function(err){
            if (err) console.log(err);
            if (DEBUG) console.log('sensor parsed = ', sensor);
            console.log('sensor ' + sensor._id +' saved Local Only');
            syncInflux(sensor)
         }) 
      }
   });
}

const Influx = require('influx')
const FieldType = Influx.FieldType;
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
         user :         FieldType.STRING,
        timestamp :  FieldType.timestamp,
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

function syncInflux(sensor) {
   console.log('syncInflux start... ');
   influx.getDatabaseNames()
  .then(names => {
    if ( !names.includes('dataspiru') ) {
      console.log('First connection... create database dataspiru');  
      return influx.createDatabase('dataspiru')
    }
  })
  .then(() => {
   console.log('Connected to InFlux');
   console.log('writePoints to Influx...');
   jsonRecord = sensor.state.reported.toJSON()

   if (DEBUG) console.log('jsonRecord = '+ JSON.stringify(jsonRecord) );         
   influx.writePoints([
      {
      measurement: 'sensor',
      tags: { sensor: "TamataSpiru" },
      fields: { 
         user :            jsonRecord.user,
         timestamp :       Date.parse(jsonRecord.timestamp),
         mac:           jsonRecord.mac,
         visibleLight:     jsonRecord.visibleLight,
         infraRed:         jsonRecord.infraRed,
         ultraViolet:      jsonRecord.ultraViolet,
         Temperature:      jsonRecord.Temperature,
         Pressure:         jsonRecord.Pressure,
         Humidity:         jsonRecord.Humidity,
         Vbat:             jsonRecord.Vbat,
         soilMoisture:     jsonRecord.soilMoisture,
         TempMC:           jsonRecord.TempMC,
         RGBSensor_r:      jsonRecord.RGBSensor_r,
         RGBSensor_g:      jsonRecord.RGBSensor_g,
         RGBSensor_b:      jsonRecord.RGBSensor_b,
         RGBSensor_colorTemp: jsonRecord.RGBSensor_colorTemp
         }  
      }
   ]).catch(err => {
      console.error(`Error saving data to InfluxDB! ${err.stack}`)
   }).then( () => {
      console.log('Doc ID=' + sensor._id + " pushed to InFlux" );
   })
  })
  .catch(err => {
    console.error(`Error creating Influx database!`)
  });
}

/*--------------------------------------------
This script is used :
- to listen on Mqtt Update topic 
- and save message onto mongoDB
---------------------------------------------*/
var DEBUG = true;

var mqtt = require('mqtt'); //includes mqtt server 
var moment = require('moment')
var Sensor = require('./sensorSchema')
var mongoose = require('mongoose');
var mongodbURI = 'mongodb://localhost:27017/dataspiru'; 
var deviceRoot = "$aws/things/6001941D9FAA/shadow/update"; 
var db;

var mqttServer = "52.17.46.139";
var mqttTopic = "$aws/things/6001941D9FAA/shadow/update";

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
         client = mqtt.connect( mqttServer );
         client.on('connect',function(err) {
            client.publish(mqttTopic, JSON.stringify(parsedMessage) )
            sensor.remoteSaved = moment.now()
            sensor.save(function(err){
               if (err) console.log(err);
               if (DEBUG) console.log('sensor parsed = ', sensor);
               console.log(sensor._id +' saved Local & Remote');
            });
         })
      } else {
         if (DEBUG) console.log('... Internet no connection !');
         sensor.save(function(err){
            if (err) console.log(err);
            if (DEBUG) console.log('sensor parsed = ', sensor);
            console.log('sensor ' + sensor._id +' saved Local Only');
         });
      }

      //*********************************
      // Setting here to Sync influxDB...
      //*********************************
      if (DEBUG) console.log('influxDB Sync process...');
      

   });
}

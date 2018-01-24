var mqtt = require('mqtt'); //includes mqtt server 
var moment = require('moment')
var Sensor = require('./sensorSchema')
var mongoose = require('mongoose');

var mongodbURI = 'mongodb://localhost:27017/dataspiru'; 
var deviceRoot = "$aws/things/6001941D9FAA/shadow/update"; 
var db;

mongoose.connect(mongodbURI, function (err, db) {
   if(err) throw console.log(err);
   console.log('connected to db begin... ');   
   client = mqtt.connect('mqtt://10.3.141.1');
   client.subscribe("$aws/things/6001941D9FAA/shadow/update"); 
   client.on('message', insertEvent);                    
});

//function that record the data in the MongoDataBase 
// Check && Internet available ? send to Mqtt AWS 
function insertEvent(topic,message) {
   console.log('message ' + message ) ;
   var parsedMessage = JSON.parse(message);

   console.log('parsedMessage' + parsedMessage);
   console.log('-----------------------------');
   var sensor = new Sensor ({
      state:{
         reported:{
            user : parsedMessage.state.reported.user,
            timestamp : parsedMessage.state.reported.timestamp,
            mac: parsedMessage.state.reported.mac,
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

   console.log('sensorNow = ', sensor);
   /* Checking Internet Connection */
    var exec = require('child_process').exec, child;
   child = exec('ping -c 1 google.com', function(error, stdout, stderr){
   //require('dns').lookupService('8.8.8.8', 53, function(err, hostname, service){   
   //require('dns').lookup('google.com',function(err) {
   //require('dns').resolve('www.Google.fr', function(err){
      if (error === null) {
         console.log('connected !!! ');
         /* Sending to Mqtt */
         client = mqtt.connect('mqtt://10.3.141.1');
         client.on('connect',function(err) {
            client.publish('dev/update', JSON.stringify(parsedMessage) )
            sensor.remoteSaved = moment.now()
            sensor.save(function(err){
               if (err) console.log(err);
               console.log(sensor._id +' saved Local & Remote');
            });
         })
      } else {
         sensor.save(function(err){
            if (err) console.log(err);
            console.log('sensor ' + sensor._id +' saved Local Only');
         });
      }

      
   });
   

}

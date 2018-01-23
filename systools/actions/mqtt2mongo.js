var mqtt = require('mqtt'); //includes mqtt server 
var moment = require('moment')
var Sensor = require('./sensorSchema')
var mongoose = require('mongoose');

var mongodbURI = 'mongodb://localhost:27017/dataspiru'; //activating the MongoDB port 27017, here TempMontor is the name of the database
var deviceRoot = "$aws/things/6001941D9FAA/shadow/update"; //deviceroot is topic name given in arduino code 
var db;

mongoose.connect(mongodbURI, function (err, db) {
   if(err) throw console.log(err);
   console.log('connected to db begin... ');   
   client = mqtt.connect('mqtt://10.3.141.1');
   client.subscribe("$aws/things/6001941D9FAA/shadow/update"); //subscribing to the topic name 
   client.on('message', insertEvent); //inserting the event
});

//function that record the data in the MongoDataBase
function insertEvent(topic,message) {
   console.log('message ' + message ) ;
   var parsedMessage = JSON.parse(message);

   console.log('parsedMessage' + parsedMessage);
   console.log('-----------------------------');
   var sensor = new Sensor ({
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
   });

   console.log('sensorNow = ', sensor);
   sensor.save(function(err){
         if (err) console.log(err);
         console.log('sensor ' + sensor._id +' succefully saved');
   });
}

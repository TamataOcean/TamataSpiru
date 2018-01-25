/*--------------------------------------------
This script is used :
- to take unpushed document from MongoDB 
- to send them to Mqtt Broker on Cloud serveur
---------------------------------------------*/
var DEBUG = true;

var mqtt = require('mqtt'); //includes mqtt server 
var moment = require('moment')
var Sensor = require('./sensorSchema')
var mongoose = require('mongoose')
var _ = require('lodash');

var mongodbURI = 'mongodb://localhost:27017/dataspiru'; 
// var topicUpdate = "$aws/things/6001941D9FAA/shadow/update"; 
// var mqttServer = "52.17.46.139";

var topicUpdate = 'dev/update'; 

mongoose.connect(mongodbURI).then( 
   () => {
      if (DEBUG) console.log('connection Ok !');
      client = mqtt.connect('mqtt://10.3.141.1');
      client.on('connect',function(err) {
         
         //Find data not yet pushed to internet
         Sensor.find( {remoteSaved:null} , function (err, sensors){ 
            if (err) console.log(err);
            
            if (DEBUG) console.log('found some records... ');
            for (var key in sensors ) {
               //console.log('push for '+ sensors[key] );
               jsonRecord = sensors[key].toJSON()
               objJson = {}
               
               for (var i in jsonRecord ) {
                  // console.log('i=' + i +' / jsonRecord = ' + jsonRecord[i]);
                  _.set(objJson,"state.reported."+i, jsonRecord[i] )
               }

               client.publish(topicUpdate, JSON.stringify(objJson), function(err){
                  if (err) console.log(err);

                  sensors[key].remoteSaved = moment.now()
                  sensors[key].save(function(err){
                     if (err) console.log(err);
                     console.log('sensor ' + sensors[key]._id +' pushed to remote Mqtt');
                  });
               });
            }
         
            client.end();
            if (DEBUG) console.log('client.end()');
         })
         .then(
            () => {
               if (DEBUG) console.log('end Find... ');
               mongoose.connection.close();
            },
            err => {console.log('Error on Find = ' + err );})
      })
   },
   err => {console.log('Error on connect = ' + err);} 
);


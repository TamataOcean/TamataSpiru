var mqtt = require('mqtt'); //includes mqtt server 
var moment = require('moment')
var Sensor = require('./sensorSchema')
var mongoose = require('mongoose')
var _ = require('lodash');

var mongodbURI = 'mongodb://localhost:27017/dataspiru'; //activating the MongoDB port 27017, here TempMontor is the name of the database
var deviceRoot = "$aws/things/6001941D9FAA/shadow/update"; //deviceroot is topic name given in arduino code 

mongoose.connect(mongodbURI).then( 
   () => {
      console.log('connection Ok !');
      client = mqtt.connect('mqtt://10.3.141.1');
      client.on('connect',function(err) {
                                                                     // DEV - Sensor.find( {_id : "5a67c551067709035c7cee7e" } , function (err, sensors){
         Sensor.find( {remoteSaved:null} , function (err, sensors){ 
            //Find data not yet pushed to internet
            if (err) console.log(err);
            console.log('found some records... ');
            for (var key in sensors ) {
               //console.log('push for '+ sensors[key] );
               jsonRecord = sensors[key].toJSON()
               objJson = {}
               
               for (var i in jsonRecord ) {
                  // console.log('i=' + i +' / jsonRecord = ' + jsonRecord[i]);
                  _.set(objJson,"state.reported."+i, jsonRecord[i] )
               }

               client.publish('dev/update', JSON.stringify(objJson), function(err){
                  if (err) console.log(err);

                  sensors[key].remoteSaved = moment.now()
                  sensors[key].save(function(err){
                     if (err) console.log(err);
                     console.log('sensor ' + sensors[key]._id +' pushed to remote Mqtt');
                  });
               });
            }
         
            client.end();
            console.log('client.end()');
         })
         .then(
            () => {
               console.log('end Find... ');
               mongoose.connection.close();
            },
            err => {console.log('Error on Find = ' + err );})
      })
   },
   err => {console.log('Error on connect = ' + err);} 
);


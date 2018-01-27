/*--------------------------------------------
This script is used :
- to listen on Mqtt Update topic 
- and save message onto mongoDB
---------------------------------------------*/
var DEBUG = false;

var mqtt = require('mqtt'); //includes mqtt server 
var moment = require('moment')

var mongoose = require('mongoose');
var Sensor = require('./sensorSchema')
var JetPack = require('./jetPackSchema')
var mongodbURI = 'mongodb://localhost:27017/dataspiru'; 
var deviceRoot = "$aws/things/6001941D9FAA/shadow/update"; 
var db;
const Influx = require('influx')
var influx = new Influx.InfluxDB();

const FieldType = Influx.FieldType;

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
   if (DEBUG) console.log('************************');
   if (DEBUG) console.log('Mqtt Message received : ');
   if (DEBUG) console.log('Insert Message : ' + JSON.stringify(parsedMessage) ) ;
   
   /* Parsing Message */
   if ( parsedMessage.state.reported.user === "TamataSpiru" ) {
      if (DEBUG) console.log('Sensor Data Parsing');
      var measurement = "sensor"
      var mqttJsonObject = new Sensor ({
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
   }
   else if (parsedMessage.state.reported.Act0 !== null ) {
      if (DEBUG) console.log('Jetack Data Parsing');
      console.log('Act7 = ' + parsedMessage.state.reported.Act7 );
      console.log('parsedMessage = '+ JSON.stringify(parsedMessage.state.reported)  );
      var measurement = "jetpack"
      if (parsedMessage.state.reported.Act7 ==='true' ) var value7 = true;
      var mqttJsonObject = new JetPack ({
         state:{
            reported:{
               Act0: parsedMessage.state.reported.Act0,
               Act1: parsedMessage.state.reported.Act1,
               Act2: parsedMessage.state.reported.Act2,
               Act3: parsedMessage.state.reported.Act3,
               Act4: parsedMessage.state.reported.Act4,
               Act5: parsedMessage.state.reported.Act5,
               Act6: parsedMessage.state.reported.Act6,
               Act7: parsedMessage.state.reported.Act7
            } 
         }
      });  
   } else {
      if (DEBUG) console.log('Message type not yet managed...');
   }
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
            mqttJsonObject.remoteSaved = moment.now()
            mqttJsonObject.save(function(err){
               if (err) console.log(err);
               if (DEBUG) console.log('sensor parsed = ', mqttJsonObject);
               console.log(mqttJsonObject._id +' saved Local & Remote');
               syncInflux(message, measurement )
            })
         })
      } else {
         if (DEBUG) console.log('... Internet no connection !');
         mqttJsonObject.save(function(err){
            if (err) console.log(err);
            if (DEBUG) console.log('sensor parsed = ', mqttJsonObject);
            console.log('sensor ' + mqttJsonObject._id +' saved Local Only');
            // syncInflux(mqttJsonObject)
            syncInflux(message, measurement)
         }) 
      }
   });
}

function syncInflux( mqttJsonObject, measurement ) {
   if (DEBUG) console.log('*********** SyncInflux for '+ measurement );
   if (measurement === "sensor") {
      if (DEBUG) console.log('syncInflux : Adding Sensor point');
         
      const influx = new Influx.InfluxDB({
        database: 'dataspiru',
        host: 'localhost',
        port: 8086,
        username: 'test',
        password: 'test',
        schema: [
          {
            measurement: measurement,
            measurement: measurement,
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

      pushDoc2Influx( influx, mqttJsonObject, measurement );
   }

   else if (measurement === "jetpack" ) {
      if (DEBUG) console.log('syncInflux : Adding jetpack point ');         
      const influx = new Influx.InfluxDB({
        database: 'dataspiru',
        host: 'localhost',
        port: 8086,
        username: 'test',
        password: 'test',
        schema: [
          {
            measurement: measurement,
            fields: {
               Act0: FieldType.BOOLEAN,
               Act1: FieldType.BOOLEAN,
               Act2: FieldType.BOOLEAN,
               Act3: FieldType.BOOLEAN,
               Act4: FieldType.BOOLEAN,
               Act5: FieldType.BOOLEAN,
               Act6: FieldType.BOOLEAN,
               Act7: FieldType.BOOLEAN
            },
            tags: [ 'jetpack' ]
          }
        ]
      });

      pushDoc2Influx( influx, mqttJsonObject, measurement );
   }
   else {
      console.log('syncInflux Error : not a managed mqtt message...');
      console.log('Msg received : '+ JSON.stringify(mqttJsonObject) );
   }
}

function pushDoc2Influx( influx, mqttJsonObject, measurement ) {
   if (DEBUG) console.log('pushDoc2Influx function...');
   influx.getDatabaseNames()
   .then(names => {
    if ( !names.includes('dataspiru') ) {
      if (DEBUG) console.log('First connection... create database dataspiru');  
      return influx.createDatabase('dataspiru')
    }
   })
   .then( () => {
      var jsonRecord = JSON.parse(mqttJsonObject);
      if (DEBUG) console.log('database : dataspiru found');
      if (DEBUG) console.log('jsonRecord = '+ JSON.stringify(jsonRecord) );

      if (measurement ==="sensor" ){
         influx.writePoints([
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
                  console.log('Doc pushed to InFlux' );
                  console.log('\n');
               });
      } 
      else if (measurement ==="jetpack" ){
         influx.writePoints([
               {
               measurement: measurement,
               tags: { jetpack: "CoolBoardJetpack" },
               fields: { 
                  Act0: jsonRecord.state.reported.Act0,
                  Act1: jsonRecord.state.reported.Act1,
                  Act2: jsonRecord.state.reported.Act2,
                  Act3: jsonRecord.state.reported.Act3,
                  Act4: jsonRecord.state.reported.Act4,
                  Act5: jsonRecord.state.reported.Act5,
                  Act6: jsonRecord.state.reported.Act6,
                  Act6: jsonRecord.state.reported.Act6,
                  Act7: jsonRecord.state.reported.Act7
                  }  
               }]).catch(err => {
                  console.error(`Error saving Jetpack data to InfluxDB! ${err.stack}`);
                  return;
               }).then( () => {
                  console.log('Doc pushed to InFlux');
                  console.log('\n');
               });
      } else 
      {
         console.log('Mqtt message Type not managed... ! ');
      }


   })
   .catch(err => {
       console.error(`Error creating Influx database!`)
       console.log(`${err.stack}`);
       return;
   });
}

var DEBUG = false;

var mongoose = require('mongoose');
var moment = require('moment');
var sensorSchema = mongoose.Schema({
   created:             {type : Date, default: Date.now},
   remoteSaved :        {type : Date, default: null },
   state:{
      reported:{
         user:                {type : String }, 
         timestamp:           {type : Date },
         mac:                 {type : String },
         visibleLight:        {type : Number },
         infraRed:            {type : Number },
         ultraViolet:         {type : Number },
         Temperature:         {type : Number },
         Pressure:            {type : Number },
         Humidity:            {type : Number },
         Vbat:                {type : Number },
         soilMoisture:        {type : Number },
         TempMC:              {type : Number },
         RGBSensor_r:         {type : Number },
         RGBSensor_g:         {type : Number },
         RGBSensor_b:         {type : Number },
         RGBSensor_colorTemp: {type : Number }
      }
   }
});

// Extracting mongodb field before exporting to Mqtt ( cloud ).
sensorSchema.options.toJSON = {
    transform: function(doc, ret, options) {
        // ret.id = ret._id;
        delete ret.created;
        delete ret.remoteSaved;
        delete ret._id;
        delete ret.__v;
        return ret;
    }
};

sensorSchema.pre('save', function(next) {
   var now = moment.now();
   if (DEBUG) console.log('presaved function...');
   if (!this.created) {
      this.created = now;
   }
   next();
});

var Sensor = mongoose.model('sensors', sensorSchema );
module.exports = Sensor;
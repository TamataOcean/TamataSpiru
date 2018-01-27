var DEBUG = false;

var mongoose = require('mongoose');
var moment = require('moment');

var mongoose = require('mongoose');
var moment = require('moment');
var jetPack = mongoose.Schema({
  state:{
    reported:{
       Act0: { type: Boolean, default:false }, // comment:{type:String, default:null}},
       Act1: { type: Boolean, default:false }, // comment:{type:String, default:null}},
       Act2: { type: Boolean, default:false }, // comment:{type:String, default:null}},
       Act3: { type: Boolean, default:false }, // comment:{type:String, default:"Heater"}},
       Act4: { type: Boolean, default:false }, // comment:{type:String, default:"Perilstatic Pump"}},
       Act5: { type: Boolean, default:false }, // comment:{type:String, default:null}},
       Act6: { type: Boolean, default:false }, // comment:{type:String, default:null}},
       Act7: { type: Boolean, default:false } // comment:{type:String, default:"Bubler"}},
    }
  },
  created:             {type : Date, default: Date.now},
  remoteSaved :        {type : Date, default: null }
});

// Extracting mongodb field before exporting to Mqtt ( cloud ).
jetPack.options.toJSON = {
    transform: function(doc, ret, options) {
        // ret.id = ret._id;
        delete ret.created;
        delete ret.remoteSaved;
        delete ret._id;
        delete ret.__v;
        return ret;
    }
};

jetPack.pre('save', function(next) {
   var now = moment.now();
   if (DEBUG) console.log('JetPack - presaved function...');
   if (!this.created) {
      this.created = now;
   }

   console.log('JetPack Object ' + JSON.stringify(this) );
   next();
});

var JetPack = mongoose.model('jetpacks', jetPack );
module.exports = JetPack;

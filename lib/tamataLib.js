var i2c = require('i2c');
var address = 0x04;
var deviceName = '/dev/i2c-1';

/* Tamata Lib to get Sensor information */
var wire = new i2c(address, {device: deviceName});
var temperature = 0;
var red=0;
var green=0;
var blue=0;
var recup=false;

wire.on('data', function(data) {
        console.log(data)
  // result for continuous stream contains data buffer, address, length, timestamp
});

/*------------ get Temperature ------------*/
/*---------------------------------------*/
var getTemperature = function(){
	wire.writeByte(0x02, function(err) {});
	wire.readBytes(0x01, 6, function(err, res) {
			temperature = res.toString('ascii');
	        console.log("tamataLib.js :" +temperature)
	});
}

/*------------ get RED color ------------*/
/*---------------------------------------*/
var getRed = function() {
	wire.writeByte(0x03, function(err) {});
	wire.readBytes(0x01, 6, function(err, res) {
			red = res.toString('ascii');
	//        console.log(red)
	});
}

/*------------ get GREEN color ------------*/
/*-----------------------------------------*/
var getGreen = function() {
	wire.writeByte(0x04, function(err) {});
	wire.readBytes(0x01, 6, function(err, res) {
			green = res.toString('ascii');
	//        console.log(green)
	});
}

/*------------ get BLUE color ------------*/
/*----------------------------------------*/
var getBlue = function(){
	wire.writeByte(0x05, function(err) {});
	wire.readBytes(0x01, 6, function(err, res) {
			blue = res.toString('ascii');
	//        console.log(blue)
	});
}

exports.getTemperature = getTemperature;
exports.getRed = getRed;
exports.getGreen = getGreen;
exports.getBlue= getBlue;


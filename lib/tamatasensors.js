/*-------- Tamata Lib to get Sensor information --------*/
/* Manage Sensors from CoolCo Board & Arduino 			*/
/*------------------------------------------------------*/

var i2c = require('i2c');
var address = 0x04;
var deviceName = '/dev/i2c-1';
var wire = new i2c(address, {device: deviceName});
wire.on('data', function(data) {
        console.log(data)
  // result for continuous stream contains data buffer, address, length, timestamp
});

var i2c = require('i2c');
var address = 0x04;
var deviceName = '/dev/i2c-1';
var wire = new i2c(address, {device: deviceName});


/* Tamata Lib to get Sensor information */
var temperature = 0;
var red=0;
var green=0;
var blue=0;
var recup=false;

/*------------ get Temperature ------------*/
/*---------------------------------------*/
var getTemperature = function(){
	wire.writeByte(0x02, function(err) {});
	wire.readBytes(0x01, 6, function(err, res) {
			temperature = res.toString('ascii');
			console.log("Temp MC :" +temperature)
			wire.close();
	});
	return temperature;
}

/*------------ get RED color ------------*/
/*---------------------------------------*/
var getRed = function() {
	wire.writeByte(0x03, function(err) {});
	wire.readBytes(0x01, 6, function(err, res) {
			red = res.toString('ascii');
			console.log('r='+red);
			wire.close();
	});
	return red;
}

/*------------ get GREEN color ------------*/
/*-----------------------------------------*/
var getGreen = function() {
	wire.writeByte(0x04, function(err) {});
	wire.readBytes(0x01, 6, function(err, res) {
			green = res.toString('ascii');
			console.log('g='+green);
			wire.close();
	});
	return green;
}

/*------------ get BLUE color ------------*/
/*----------------------------------------*/
var getBlue = function(){
//	wire.on('data',function(data) {})
	wire.writeByte(0x05, function(err) {});
	wire.readBytes(0x01, 6, function(err, res) {
			blue = res.toString('ascii');
			console.log('b='+blue);
			wire.close();
	});
	
	return blue;
}

exports.getRed = getRed;
exports.getGreen = getGreen;
exports.getBlue= getBlue;
exports.getTemperature = getTemperature;


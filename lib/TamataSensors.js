# Manage Sensors from CoolCo Board & Arduino 

function getTemperature(wire, objJSON) {
	wire.writeByte(0x02, function(err){}); 
	wire.readBytes(0x01, 6, function(err, res) 
	{
		temperature = parseFloat(res.toString('ascii'),10);
		objJSON.state.reported.Temp = temperature;
		//console.log('temp :'+temperature);
		msgCount++;
		finale();
	});
}


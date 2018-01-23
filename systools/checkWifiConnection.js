require('dns').resolve('www.Google.fr', function(err){
	if (err) {
		console.log("No connection");
	} else {
		console.log('connected !!! ');
	}
});

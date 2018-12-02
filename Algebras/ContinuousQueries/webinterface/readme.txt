Deploy on any web server with php.

In "./index.html" change

	window.sspglobals = {
	   apiurl: "http://localhost/wss/api/api.php"
	}

to the correct url.

In "./api/api.php" change

	$address = "127.0.0.1";
	$port = 12300;

to the ip and port of the coordinator.

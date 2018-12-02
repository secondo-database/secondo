<?php
// Setup the address and the port of the SSP Coordinator.

$address = "127.0.0.1";
$port = 12300;


// Set header
header('Access-Controll-Allow-Origin: *');
header('Content-type: application/json; charset=utf-8');

// Helper function for errors
function err($reason, $exit=true) {
    echo "{\"error\": \"" . $reason . "\"}";
    if ($exit) exit;
}

// Helper function to convert the return of the Coordinator to JSON
function printSSPtoJSON($msg) {
    echo json_encode(explode("|", $msg));
}

// Check and choose Command
if ($_GET["cmd"]) {
    $cmd = $_GET["cmd"];
    if (!in_array($cmd, ["userauth", "getqueries", "addquery", "deletequery"])) {
        err("Wrong command!");
    };
} else {
    err("No command. Exit!");
}

if ($cmd == "userauth") {
    if (!$_GET["hash"] || !$_GET["email"] || !$_GET["type"] ) {
        err("Parameters missing.");
    } 

    if (!in_array($_GET["type"], ["login", "register"])) {
        err("Wrong type!");
    };

    $msg = "userauth|" . $_GET["hash"] . "|" . $_GET["email"] . "|" . $_GET["type"];
}

if ($cmd == "getqueries") {
    if (!$_GET["hash"]) {
        err("Parameters missing.");
    } 

    $msg = "getqueries|" . $_GET["hash"];
}

if ($cmd == "addquery") {
    if (!$_GET["hash"] || !$_GET["function"]) {
        err("Parameters missing.");
    } 

    $msg = "addquery|" . $_GET["hash"] . "|" . $_GET["function"];
}

// Open Socket
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);   
if ($socket === false) {
    err("socket_create() failed: reason: " . socket_strerror(socket_last_error()));
}
    
$result = socket_connect($socket, $address, $port);
if ($result === false) {    
    err("socket_connect() failed. Reason: " . socket_strerror(socket_last_error($socket)));
}

// Execute API call
socket_write($socket, $msg, strlen($msg));


// socket_set_option($socket, SOL_SOCKET,SO_RCVTIMEO, array("sec"=>1,"usec"=>0));

$r = socket_read($socket, 4096, PHP_NORMAL_READ);

echo "{\"result\": [";

while (strlen(trim($r)) > 0) {
    echo json_encode(explode("|", trim($r)));
    $r = socket_read($socket, 4096, PHP_NORMAL_READ);
    if (strlen(trim($r)) > 0) echo ",";
}

echo "]}";

// Close Socket
socket_close($socket);
?>
<?
require_once('globals.php');
require_once('redis.php');

$gRedisError = "";
$gRedisConnect = NULL;

function RedisErrorHandler($errno, $errstr, $errfile, $errline){
    switch ($errno) {
    case E_USER_ERROR:
        $gRedisError .= $errstr;
        header("HTTP/1.0 200 OK");
        echo $errstr;
        exit(0);

    case E_USER_WARNING:
    case E_USER_NOTICE:
    }
    /* Don't execute PHP internal error handler */
    return true;
}
set_error_handler("RedisErrorHandler");

function GetRedisConnection()
{
    global $gRedisError, $gRedisHost, $gRedisPort, $gRedisAuth, $gRedisConnect;
    
    $gRedisError = "";
    if (is_null($gRedisConnect)) {
        //create connection
        $gRedisConnect = new Redis($gRedisHost, $gRedisPort);
        if ($redisPass != '') {
            // perfofm authentication
        }
    }
    if ($gRedisError != "") {
        $gRedisConnect = NULL;
    }
    return $gRedisConnect;
}


function SetValue($rconn, $key, $value) {
    $msg = $rconn->set($key, $value);
    if ($msg != "OK") {
        trigger_error("Can't set $key: $msg.", E_USER_ERROR);
    }
}

?>
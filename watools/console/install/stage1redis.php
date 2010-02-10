<?
require_once('../redis.php');

$result = "";

function RedisErrorHandler($errno, $errstr, $errfile, $errline){
    switch ($errno) {
    case E_USER_ERROR:
        $result .= $errstr;
        echo $result;
        exit(1);

    case E_USER_WARNING:
    case E_USER_NOTICE:
    }
    /* Don't execute PHP internal error handler */
    return true;
}
set_error_handler("RedisErrorHandler");

header("HTTP/1.0 200 OK");

$r =& new Redis('10.0.0.1'); // host, port 
$r->connect();
$r->select_db(0); // DB index
//$r->flushdb();

if ($result == "") {
    $result = "SUCCESS";
}
echo $result;
?>
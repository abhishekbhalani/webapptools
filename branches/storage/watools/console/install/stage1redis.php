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

function SetValue($rconn, $key, $value) {
    $msg = $rconn->set($key, $value);
    if ($msg != "OK") {
        trigger_error("Can't set $key: $msg.", E_USER_ERROR);
    }
}

header("HTTP/1.0 200 OK");

$r =& new Redis(); // host, port 
$r->connect();
$r->select_db(0); // DB index
$r->flushdb();
SetValue($r, 'Global:UserID', 0);
SetValue($r, 'Global:GroupID', 0);
SetValue($r, 'Global:ThemeID', 0);

if ($result == "") {
    $result = "SUCCESS";
}
echo $result;
?>
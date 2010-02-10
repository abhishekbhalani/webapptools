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
// create group
$gid = $r->incr('Global:GroupID');
SetValue($r, "GroupName:Administrators", $id);
$msg = $r->rpush("Group:$gid", "Administrators");
if ($msg != "OK") {
    trigger_error("Can't save Group:$gid => $msg.", E_USER_ERROR);
}
$msg = $r->rpush("Group:$gid", "System administrators");
if ($msg != "OK") {
    trigger_error("Can't save Group:$id => $msg.", E_USER_ERROR);
}

// create user
$id = $r->incr('Global:UserID');
SetValue($r, "Login:Administrator", $gid);
$msg = $r->rpush("User:$id", "Administrator");
if ($msg != "OK") {
    trigger_error("Can't save User:$id => $msg.", E_USER_ERROR);
}
$msg = $r->rpush("User:$id", "System administrator");
if ($msg != "OK") {
    trigger_error("Can't save User:$id => $msg.", E_USER_ERROR);
}
$pwd = md5("");
$msg = $r->rpush("User:$id", $pwd);
if ($msg != "OK") {
    trigger_error("Can't save User:$id => $msg.", E_USER_ERROR);
}

// add user to group
$msg = $r->sadd("User:$id:Groups", $gid);
if ($msg == 0) {
    trigger_error("Can't save User:$id:Groups => $msg.", E_USER_ERROR);
}
$msg = $r->sadd("Group:$gid:Members", $id);
if ($msg == 0) {
    trigger_error("Can't save Group:$id:Members => $msg.", E_USER_ERROR);
}


if ($result == "") {
    $result = "SUCCESS";
}
echo $result;
?>
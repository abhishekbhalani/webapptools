<?
require_once('redisDB.php');
require_once('usermgmt.php');

$gSession = NULL;
$gUser = NULL;

$r = GetRedisConnection();

if (is_null($r)) {
    header("HTTP/1.0 302 Found");
    header("Location: /maintance.php");
    exit(0);
}

// detect theme settings
$theme = '';
if (isset($_COOKIE['WATTHEME'])) {
    $theme = $_COOKIE['WATTHEME'];
}
if ($theme == '') {
    $theme = $gDefaultTheme;
}

// detect language settings
$lang = $r->get('Global:DefaultLang');;
if (isset($_COOKIE['WATLANG'])) {
    $lang = $_COOKIE['WATLANG'];
}
else {
    if (isset($_SERVER['HTTP_ACCEPT_LANGUAGE'])) {
        $slangs = explode(',',$_SERVER['HTTP_ACCEPT_LANGUAGE']);
        $lang = $slang[0];
        $lang = substr($lang,0,2);
    }
}
if ($lang == '') {
    $lang = 'en';
}

if (isset($_COOKIE['WATSESSION'])) {
    $sid = $_COOKIE['WATSESSION'];
    if ($r->sismember("Sessions", $sid) == 0) {
        $r->delete("Session:$sid");
        // fall throught to the new session creation
    }
    else {
        $gSession = $r->lrange("Session:$sid", 0, 5);
        $gSession['id'] = $sid;
        // check for timeout
        $stm = $gSession[0];
        $ltm = time();
        if ($stm < $ltm) {
            // 5 minutes timeout expired
            $r->delete("Session:$sid");
            $r->srem("Sessions", $sid);
            $gSession = NULL;
        }
        else {
            // update timeout value
            $gSession[0] = $ltm + 300;
            $r->lset("Session:$sid", $gSession[0], 0);
        }
        // check session hijacking attempt
        if (!is_null($gSession)) {
            if ($gSession[2] != $_SERVER['REMOTE_ADDR']) {
                // atatat!!!
                $r->delete("Session:$sid");
                $r->srem("Sessions", $sid);
                $gSession = NULL;
            }
        }
    }
}

// check for cleanup
$stm = $r->get('Global:SessionCheck');
$ltm = time();
if ($stm < $ltm) {
    CleanUpSessions();
    $stm = $ltm + 600; // 10 minutes;
    $r->set('Global:SessionCheck', $stm);
}

if (is_null($gSession)) {
    // create new session
    $ltm = time();
    $sid = md5($ltm . $_SERVER['HTTP_USER_AGENT'] . $_SERVER['REMOTE_ADDR']);
    while ($r->sismember("Sessions", $sid) == 1) {
        $sid .=  $_SERVER['REMOTE_ADDR'] . rand(0, 9);
        $sid = md5($sid);
    }
    $r->sadd("Sessions", $sid);
    $ltm += 300;
    $r->rpush("Session:$sid", $ltm);             // timeout 0
    $r->rpush("Session:$sid", -1);               // ClientID 1
    $addr = $_SERVER['REMOTE_ADDR'];
    $r->rpush("Session:$sid", $addr);            // ConnectionID 2
    // get theme
    $theme = $r->get('Global:DefaultTheme');
    if (isset($_COOKIE['WATTHEME'])) {
        $theme = $_COOKIE['WATTHEME'];
    }
    $r->rpush("Session:$sid", $theme);           // Theme 3
    $r->rpush("Session:$sid", $lang);            // Language 4
    // return session
    $gSession = $r->lrange("Session:$sid", 0, 5);
    $gSession['id'] = $sid;
    $r->sadd("Sessions", $sid);
}

if (!is_null($gSession)) {
    // save session
    setcookie('WATSESSION', $gSession['id'], $gSession[0]);
    setcookie('WATTHEME', $theme);
    setcookie('WATLANG', $lang);

    // check for authentication mode
    $gUser = GetUserByID($gSession[1]);
}

// functions for extarnal use
function CleanUpSessions() {
    $r = GetRedisConnection();
    if (!is_null($r)) {
        $alls = $r->keys("Session:*");
        $ltm = time();
        foreach ($alls as $sess) {
            $sid = substr($sess,8);
            if ($r->sismember("Sessions", $sid) == 0) {
                // delete broken session
                DeleteSession($r, $sid);
            }
            else {
                $s = $r->lrange("Session:$sid", 0, 5);
                if ($s[0] < $ltm) {
                    DeleteSession($r, $sid);
                }
            }
        }
    }
    return true;
}

function SaveSession() {
    global $gSession;
    
    $r = GetRedisConnection();
    if (!is_null($r)) {
        $ltm = time();
        if ($gSession[0] < $ltm) {
            DeleteSession($r, $gSession['id']);
            $gSession = NULL;
            return;
        }
        if ($r->sismember("Sessions", $gSession['id']) == 0) {
            // delete broken session
            DeleteSession($r, $gSession['id']);
            $gSession = NULL;
            return;
        }
        // fall throught to the new session creation
        $key = "Session:" . $gSession['id'];
        $r->lset($key, $gSession[0], 0);
        $r->lset($key, $gSession[1], 1);
        $r->lset($key, $gSession[2], 2);
        $r->lset($key, $gSession[3], 3);
        $r->lset($key, $gSession[4], 4);
    }
}

function DeleteSession($redis, $sid) {
    $redis->srem("Sessions", $sid);
    $redis->delete("Session:$sid");
    return true;
}
?>

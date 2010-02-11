<?
require_once('redisDB.php');
require_once('usermgmt.php');

$gSession = NULL;
$gUser = NULL;

ob_start();
echo "<pre>";

$r = GetRedisConnection();

if (is_null($r)) {
    header("HTTP/1.0 302 Found");
    header("Location: /maintance.php");
    exit(0);
}

if (isset($_COOKIE['WATSESSION'])) {
    $sid = $_COOKIE['WATSESSION'];
    if ($r->sismember("Sessions", $sid) == 0) {
echo "No such :(\n";
print_r($r->smembers("Sessions"));
print_r($r->keys("Session:*"));
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
echo "timeout $stm < $ltm\n";
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
echo "atatat" . $gSession[2] . "!=" . $_SERVER['REMOTE_ADDR'] . "\n";
                // atatat!!!
                $r->delete("Session:$sid");
                $r->srem("Sessions", $sid);
                $gSession = NULL;
            }
        }
    }
}

if (is_null($gSession)) {
    // create new session
echo "new session\n";

    $ltm = time();
    $sid = md5($ltm . $_SERVER['HTTP_USER_AGENT'] . $_SERVER['REMOTE_ADDR']);
    while ($r->sismember("Sessions", $sid) == 1) {
        $sid .=  $_SERVER['REMOTE_ADDR'] . rand(0, 9);
        $sid = md5($sid);
    }
    $r->sadd("Sessions", $sid);
    $ltm += 300;
    $r->rpush("Session:$sid", $ltm);             // timeout
    $r->rpush("Session:$sid", -1);               // ClientID
    $addr = $_SERVER['REMOTE_ADDR'];
    $r->rpush("Session:$sid", $addr);            // ConnectionID
    // get theme
    $theme = 'sandbox';
    if (isset($_COOKIE['WATTHEME'])) {
        $theme = $_COOKIE['WATTHEME'];
    }
    $r->rpush("Session:$sid", $theme);           // Theme
    
    // get language
    $lang = 'en';
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
    $r->rpush("Session:$sid", $lang);            // Language
    // return session
    $gSession = $r->lrange("Session:$sid", 0, 5);
    $gSession['id'] = $sid;
    $r->sadd("Sessions", $sid);
}

if (!is_null($gSession)) {
    // save session
    setcookie('WATSESSION', $gSession['id'], $gSession[0]);

    // check for authentication mode
    $gUser = GetUserByID($gSession[1]);
}

// functions for extarnal use
function SetSessionTheme() {
    return true;
}

function SetSessionLocale() {
    return true;
}

function CleanUpSessions() {
    return true;
}

function DeleteSession($sid) {
    return true;
}

print_r($gSession);
print_r($gUser);
print_r($_COOKIE);
echo "</pre>";
ob_end_flush();
?>

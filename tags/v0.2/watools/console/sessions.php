<?
header("HTTP/1.0 200 OK");
require_once('db.php');
require_once('usermgmt.php');

$gSession = NULL;
$gUser = NULL;

$r = GetDbConnection();

if (is_null($r)) {
    header("HTTP/1.0 302 Found");
    header("Location: /maintance.php");
    exit(0);
}

$tsess = GetTableName("sessions");
$tint = GetTableName("_internals_");
$ttheme = GetTableName("themes");

// detect theme settings
$theme = '';
if (isset($_COOKIE['WATTHEME'])) {
    $theme = $_COOKIE['WATTHEME'];
}
if ($theme == '') {
	$q = GetSingleRow($r, "SELECT value FROM $tint WHERE name='DefaultTheme'");
	if (!is_null($q)) {
		$theme = $q[0];
	}
}

// detect language settings
$q = GetSingleRow($r, "SELECT value FROM $tint WHERE name='DefaultLang'");
if (!is_null($q)) {
	$lang = $q[0];
}
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
	$sess = GetSingleRow($r, "SELECT * FROM $tsess WHERE id='$sid'");
    if (!is_null($sess) && $sess['id'] == $sid) {
        $gSession = $sess;
        // check for timeout
        $stm = $gSession['timeout'];
        $ltm = time();
        if ($stm < $ltm) {
            // 5 minutes timeout expired
			DbExec($r, "DELETE FROM $tsess WHERE id='$sid'");
            $gSession = NULL;
        }
        else {
            // update timeout value
			$ltm += 300;
            $gSession['timeout'] = $ltm;
			DbExec($r, "UPDATE $tsess SET timeout=$ltm WHERE id='$sid'");
        }
        // check session hijacking attempt
        if (!is_null($gSession)) {
			$client = md5($_SERVER['REMOTE_ADDR'] . $_SERVER['HTTP_USER_AGENT']);
            if ($gSession['connection_id'] != $client) {
                // atatat!!!
                DbExec($r, "DELETE FROM $tsess WHERE id='$sid'");
				$gSession = NULL;
            }
        }
    }
}

// check for cleanup
$stm = GetSingleRow($r, "SELECT value FROM $tint WHERE name='SessionCheck'");
$ltm = time();
if (is_null($stm[0])) {
	$stm = array($ltm);
	DbExec($r, "INSERT INTO $tint (name, value) VALUES('SessionCheck', $ltm)");
}
if ($stm[0] < $ltm) {
    CleanUpSessions();
    $stm[0] = $ltm + 300; // 5 minutes;
	DbExec($r, "UPDATE $tint SET value=$stm[0] WHERE name='SessionCheck'");
}

if (is_null($gSession)) {
    // create new session
    $ltm = time();
    $sid = md5($ltm . $_SERVER['HTTP_USER_AGENT'] . $_SERVER['REMOTE_ADDR']);
	$sess = GetSingleRow($r, "SELECT * FROM $tsess WHERE id='$sid'");
    while (!is_null($sess[0])) {
        $sid .=  $_SERVER['REMOTE_ADDR'] . rand(0, 9);
        $sid = md5($sid);
		$sess = GetSingleRow($r, "SELECT * FROM $tsess WHERE id='$sid'");
    }
    $ltm += 300;								 // timeout 0
    $cli_id = -1;				                 // ClientID 1
    $addr = $_SERVER['REMOTE_ADDR'] . $_SERVER['HTTP_USER_AGENT'];
    $conn_id = md5($addr);                       // ConnectionID 2
    // get theme
	$q = GetSingleRow($r, "SELECT value FROM $tint WHERE name='DefaultTheme'");
	if (is_null($q)) {
		// Theme 3
		if (isset($_COOKIE['WATTHEME'])) {
			$theme = $_COOKIE['WATTHEME'];
		}
		else {
			$theme = $gDefaultTheme;
		}
	}
	else {
		$theme = $q[0];
	}
    //$lang							             // Language 4
    // Create session
	$s = $r->prepare("INSERT INTO $tsess (id, timeout, client_id, connection_id, theme, lang) VALUES (?, ?, ?, ?, ?, ?)");
	if ($s == false) {
		PrintDbError($r);
	}
	$s->bindParam(1, $sid);
	$s->bindParam(2, $ltm);
	$s->bindParam(3, $cli_id);
	$s->bindParam(4, $conn_id);
	$s->bindParam(5, $theme);
	$s->bindParam(6, $lang);
	if ( ! $s->execute()) {
		PrintDbError($s);
	}
    $gSession = GetSingleRow($r, "SELECT * FROM $tsess WHERE id='$sid'");
}

if (!is_null($gSession)) {
    // check for authentication mode
    $gUser = GetUserByID($gSession['client_id']);
	$sid = $gSession['id'];
    // overwrite user defined parameters
    if (!is_null($gUser)) {
        $inf = $gUser['theme'];
        if (!is_null($inf) && $inf != "") {
            $theme = $inf;
			$gSession['theme'] = $theme;
			DbExec($r, "UPDATE $tsess SET theme=$theme WHERE id=$sid");
        }
        $inf = $gUser['lang'];
        if (!is_null($inf) && $inf != "") {
            $lang = $inf;
			$gSession['lang'] = $lang;
			DbExec($r, "UPDATE $tsess SET lang='$lang' WHERE id='$sid'");
        }
		SaveSession();
    }
    // save session
    setcookie('WATSESSION', $gSession['id'], $gSession['timeout']);
    setcookie('WATTHEME', $gSession['theme']);
    setcookie('WATLANG', $gSession['lang']);
}

// functions for extarnal use
function CleanUpSessions() {
    $r = GetDbConnection();
	$tsess = GetTableName("sessions");
    if (!is_null($r)) {
		$ltm = time();
		DbExec($r, "DELETE FROM $tsess WHERE timeout <= $ltm");
    }
    return true;
}

function GetLoggedUsers()
{
    $res = array();
    $r = GetDbConnection();
    if (!is_null($r)) {
        $alls = $r->query("SELECT client_id FROM $tsess");
        foreach ($alls as $sess) {
			$uid = $sess['client_id'];
            if ($uid > -1) {
				$u = GetUserByID($uid);
				if (!is_null($u)) {
					$res[] = $u[0];
				}
			}
        }
    }
    return $res;
}

function SaveSession() {
    global $gSession;
    
    $r = GetDbConnection();
	$tsess = GetTableName("sessions");
    if (!is_null($r)) {
        $ltm = time();
        if ($gSession['timeout'] < $ltm) {
            DeleteSession($r, $gSession['id']);
            $gSession = NULL;
            return;
        }
        // fall throught to the new session creation
    // Create session
	$s = $r->prepare("REPLACE INTO $tsess (id, timeout, client_id, connection_id, theme, lang) VALUES (?, ?, ?, ?, ?, ?)");
	if ($s == false) {
		PrintDbError($r);
	}
	$s->bindParam(1, $gSession['id']);
	$s->bindParam(2, $gSession['timeout']);
	$s->bindParam(3, $gSession['client_id']);
	$s->bindParam(4, $gSession['connection_id']);
	$s->bindParam(5, $gSession['theme']);
	$s->bindParam(6, $gSession['lang']);
	if ( ! $s->execute()) {
		PrintDbError($s);
	}
	$sid = $gSession['id'];
	$gSession = GetSingleRow($r, "SELECT * FROM $tsess WHERE id='$sid'");
    }
}

function DeleteSession($dbh, $sid) {
	$tsess = GetTableName("sessions");
	DbExec($dbh, "DELETE FROM $tsess WHERE id='$sid'");
    return true;
}

?>

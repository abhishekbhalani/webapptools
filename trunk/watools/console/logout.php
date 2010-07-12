<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

$r = GetDbConnection();

if (!is_null($gSession) && !is_null($r)) {
    DeleteSession($r, $gSession['id']);
}

header('HTTP/1.0 302 FOUND');
header('Location: /');
?>

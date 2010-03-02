<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

if (!is_null($gSession) && !is_null($gUser)) {
    header('HTTP/1.0 302 FOUND');
    header('Location: /main.php');
    exit(0);
}

DisplayThemePage('index.html');
?>

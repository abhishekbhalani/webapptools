<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (! CheckACL('main')) {
    header('HTTP/1.0 302 FOUND');
    header('Location: /');
}

$smarty->assign('UserName', $gUser['login']);
DisplayThemePage('main.html';
?>

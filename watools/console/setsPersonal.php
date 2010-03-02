<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('settings/personal')) {
    PrintNoAccess();
    exit(0);
}

$smarty->assign('UserName', $gUser[0]);
$smarty->assign('userName', $gUser[1]);
DisplayThemePage('settings/personal.html');
?>
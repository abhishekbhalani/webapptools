<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('settings')) {
    PrintNoAccess();
    exit(0);
}
$r = GetRedisConnection();
$isSystem = false;
$acl = GetACL('settings');
if (!is_null($r)) {
    if (in_array('system', $acl)) {
        $isSystem = true;
    }
}
$smarty->assign('UserName', $gUser[0]);
$smarty->assign('SysUser', $isSystem);
DisplayThemePage('settings.html');
?>
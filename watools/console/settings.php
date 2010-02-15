<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('dashboard')) {
    $smarty->assign('UserName', $gUser[0]);
    $smarty->assign('theme', $themeName);
    $smarty->display('noaccess.html' . $themeLangExt);
    exit(0);
}
$r = GetRedisConnection();
$isSystem = false;
if (!is_null($r)) {
    if (in_array(1, $gUser['groups']) || $gUser['id'] == 1) {
        $isSystem = true;
    }
}
$smarty->assign('UserName', $gUser[0]);
$smarty->assign('SysUser', $isSystem);
$smarty->assign('theme', $themeName);
$smarty->display('settings.html' . $themeLangExt);
?>
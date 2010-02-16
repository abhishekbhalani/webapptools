<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('dashboard')) {
    PrintNoAccess();
    exit(0);
}
$r = GetRedisConnection();
$isSystem = false;
if (!is_null($r)) {
    // check for admin group
    if (in_array(1, $gUser['groups']) || $gUser['id'] == 1) {
        $isSystem = true;
        // put system information to dashboard
        $rInfo = $r->info();
        $smarty->assign('redisVersion', $rInfo['redis_version']);
        $smarty->assign('redisUptime', $rInfo['uptime_in_days']);
        $smarty->assign('redisRole', $rInfo['role']);
        $smarty->assign('redisMemory', $rInfo['used_memory']);
        $smarty->assign('redisKeys', $rInfo['db' . $gRedisDB]);
    }
}
$smarty->assign('UserName', $gUser[0]);
$smarty->assign('SysUser', $isSystem);
$smarty->assign('theme', $themeName);
$smarty->display('dashboard.html' . $themeLangExt);
?>

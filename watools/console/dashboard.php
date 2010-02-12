<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('dashboard')) {
	$smarty->assign('UserName', $gUser[0]);
	$smarty->assign('theme', $themeName);
	$smarty->display('noaccess.html' . $themeLangExt);
}
$r = GetRedisConnection();
if (!is_null($r)) {
	$rInfo = $r->info();
	$smarty->assign('redisVersion', $rInfo['redis_version']);
	$smarty->assign('redisUptime', $rInfo['uptime_in_days']);
	$smarty->assign('redisRole', $rInfo['role']);
	$smarty->assign('redisMemory', $rInfo['used_memory']);
	$smarty->assign('redisKeys', $rInfo['db' . $gRedisDB]);
}
$smarty->assign('UserName', $gUser[0]);
$smarty->assign('theme', $themeName);
$smarty->display('dashboard.html' . $themeLangExt);
?>

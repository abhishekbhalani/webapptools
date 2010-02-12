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

$smarty->assign('UserName', $gUser[0]);
$smarty->assign('theme', $themeName);
$smarty->display('dashboard.html' . $themeLangExt);
?>

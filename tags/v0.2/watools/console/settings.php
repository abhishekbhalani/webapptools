<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

$r = GetDbConnection();
if (is_null($r)) {
    header("HTTP/1.0 302 Found");
    header("Location: /maintance.php");
    exit(0);
}

// todo check ACL for access
if (!CheckACL('settings')) {
    PrintNoAccess();
    exit(0);
}
$isSystem = false;
$acl = GetACL('settings');
if (in_array('system', $acl)) {
	$isSystem = true;
}
print "<!-- acl=";
print_r($acl);
print "-->";

$smarty->assign('UserName', $gUser['login']);
$smarty->assign('SysUser', $isSystem);
DisplayThemePage('settings.html');
?>
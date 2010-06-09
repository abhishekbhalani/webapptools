<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('settings/usersgrops')) {
    PrintNoAccess();
    exit(0);
}

$r = GetRedisConnection();
$users = array();
if (!is_null($r)) {
    $logins = $r->keys("Login:*");
    foreach ($logins as $l) {
        $l = substr($l, 6);
        $u = GetUserByName($l);
        $u[9] = $u['id'];
        $u[10] = GetUserGroups($u['id']);
        $users[] = $u;
    }
}
$groups = array();
$grpIds = array();
$grpNames = array();
if (!is_null($r)) {
    $grpnm = $r->keys("GroupName:*");
    foreach ($grpnm as $l) {
        $l = substr($l, 10);
        $u = GetGroupByName($l);
        $u[9] = $u['id'];
        $groups[] = $u;
        $grpIds[] = $u['id'];
        $grpNames[] = $u[0];
    }
}

$smarty->assign('UserName', $gUser[0]);
$smarty->assign('userList', $users);
$smarty->assign('groupList', $groups);
$smarty->assign('grpIds', $grpIds);
$smarty->assign('grpNames', $grpNames);
DisplayThemePage('settings/usergroups.html');
?>
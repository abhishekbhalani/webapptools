<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
//if (!CheckACL('settings/usersgrops')) {
//    PrintNoAccess();
//    exit(0);
//}

$db = GetDbConnection();
$users = array();
if (!is_null($db)) {
	$table = GetTableName("users");
    $logins = $db->query("SELECT login FROM $table")->fetchAll(PDO::FETCH_COLUMN);
    foreach ($logins as $l) {
        $u = GetUserByName($l);
        $grp = GetUserGroups($u['id']);
		$u[9] = $grp[0]; // "primary group"
		foreach ($grp as $g) {
			$gd = GetGroupByID($g);
			$u[10][] = $gd['name'];
		}
        $users[] = $u;
    }
}
$groups = array();
$grpIds = array();
$grpNames = array();
if (!is_null($db)) {
	$table = GetTableName("groups");
    $grpnm = $db->query("SELECT name FROM $table")->fetchAll(PDO::FETCH_COLUMN);
    foreach ($grpnm as $l) {
        $u = GetGroupByName($l);
        $groups[] = $u;
        $grpIds[] = $u['id'];
        $grpNames[] = $u['name'];
    }
}

$smarty->assign('UserName', $gUser[0]);
$smarty->assign('userList', $users);
$smarty->assign('groupList', $groups);
$smarty->assign('grpIds', $grpIds);
$smarty->assign('grpNames', $grpNames);
DisplayThemePage('settings/usergroups.html');
?>
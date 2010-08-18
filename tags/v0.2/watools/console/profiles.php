<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('dashboard')) {
    PrintNoAccess();
    exit(0);
}
$db = GetDbConnection();
$table = GetTableName("profile");
$pq = $db->query("SELECT profile_id, value FROM $table WHERE name='profile_name'");
$profiles = array();
if ($pq) {
	$profiles = $pq->fetchAll(PDO::FETCH_NUM);
}
$smarty->assign('proflist', $profiles);
DisplayThemePage('profiles.html');
?>
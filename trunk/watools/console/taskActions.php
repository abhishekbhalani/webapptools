<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

$db = GetDbConnection();
if (is_null($db)) {
	exit(0);
}
// todo check ACL for access
if (!CheckACL('tasks')) {
    PrintNoAccess();
    exit(0);
}

$acl = GetACL('tasks');
if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
    $acl = 1;
}
if ($acl == 1) {
	$action = $_POST['action'];
	$tid = $_POST['task'];
	$table = GetTableName("task");
	$tscan = GetTableName("scan_data");
	$tvuln = GetTableName("vulners");
	if ($action == "del") {
		$tid *= 2;
		$tid /= 2; // make int
		$db->exec("DELETE FROM $table WHERE id=$tid");
		$db->exec("DELETE FROM $tscan WHERE task_id=$tid");
		$db->exec("DELETE FROM $tvuln WHERE task_id=$tid");
		
		print "OK";
	}
	else if ($action == "stop") {
	}
	else {
		print gettext("Invalid request!");
	}
}
else {
	PrintNoAccess();
}
?>
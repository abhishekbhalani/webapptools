<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('tasks')) {
    PrintNoAccess();
    exit(0);
}
$db = GetDbConnection();
$table = GetTableName("task");
$tscan = GetTableName("scan_data");
$tprof = GetTableName("profile");
// class=0 - the scanners
$tlist = $db->query("SELECT * FROM $table")->fetchAll();
foreach($tlist as $task) {
	$tsk = array();
	$tsk[] = $task['id'];
	$tid = $task['id'];
	$pid = $task['profile_id'];
	$q = $db->query("SELECT value FROM $tprof WHERE profile_id=$pid AND name='profile_name'")->fetchAll();
	if ($q && !is_null($q[0])) {
		$pname = $q[0][0];
	}
	else {
		$pname = $pid;
	}
	$tsk[] = $pname;
	$tsk[] = $task['status'];
	$tsk[] = $task['completion'];
	$tsk[] = $task['start_time'];
	$tsk[] = $task['finish_time'];
	// get task target
	// SELECT object_url FROM $tscan WHERE task_id=$tid ORDER BY id ASC LIMIT 1
	$q = $db->query("SELECT object_url FROM $tscan WHERE task_id=$tid ORDER BY id ASC LIMIT 1");
	$oid = "Unknown";
	if ($q) {
		$url = $q->fetchAll(PDO::FETCH_COLUMN);
		if ($url) {
			$oid = $url[0];
			$oid = preg_replace("/[^:\\/]+:\\/\\/([^\\/]+)\\/.*/", "$1", $oid);
		}
	}
	$tsk[] = $oid;
	
	$tskList[] = $tsk;
	$dbg .= print_r($task, true);
}
$tskNum = count($tskList);
$acl = GetACL('tasks');
if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
    $acl = 1;
}

$smarty->assign('TaskACL', $acl);
$smarty->assign('TaskNum', $tskNum);
$smarty->assign('TaskList', $tskList);
DisplayThemePage('tasks.html');
?>
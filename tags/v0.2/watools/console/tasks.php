<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

$tsk_status = array();
$tsk_status[] = gettext("IDLE");
$tsk_status[] = gettext("RUNNING");
$tsk_status[] = gettext("PAUSED");
$tsk_status[] = gettext("FINISHED");
$tsk_status[] = gettext("STOPPED");
$tsk_status[] = gettext("HANGS");
// last status
$tsk_status[] = gettext("UNDEFINED");
$tsk_last = count($tsk_status) - 1;

// todo check ACL for access
if (!CheckACL('tasks')) {
    PrintNoAccess();
    exit(0);
}
$db = GetDbConnection();
$table = GetTableName("task");
$tscan = GetTableName("scan_data");
$tprof = GetTableName("profile");
$tint = GetTableName("_internals_");
$timeout = GetSingleRow($db, "SELECT value FROM $tint WHERE name='TaskTimeout'");
if ($timeout) {
	$timeout = $timeout[0];
}
else {
	$timeout = 10;
}
// class=0 - the scanners
$tlist = $db->query("SELECT * FROM $table")->fetchAll();
foreach($tlist as $task) {
	$tsk = array();
	$tsk[0] = $task['id'];
	$tid = $task['id'];
	$pid = $task['profile_id'];
	$q = $db->query("SELECT value FROM $tprof WHERE profile_id=$pid AND name='profile_name'")->fetchAll();
	if ($q && !is_null($q[0])) {
		$pname = $q[0][0];
	}
	else {
		$pname = $pid;
	}
	$tsk[1] = $pname;
	// get task target
	// SELECT object_url FROM $tscan WHERE task_id=$tid ORDER BY id ASC LIMIT 1
	$oid = $task['name'];
	if ($oid == "") {
		$q = $db->query("SELECT object_url FROM $tscan WHERE task_id=$tid ORDER BY id ASC LIMIT 1");
		if ($q) {
			$url = $q->fetchAll(PDO::FETCH_COLUMN);
			if ($url) {
				$oid = $url[0];
				$oid = preg_replace("/[^:\\/]+:\\/\\/([^\\/]+)\\/.*/", "$1", $oid);
			}
		}
	}
	$tsk[2] = $oid;
	$tsk[3] = $task['status'];
	$tsk[4] = $task['completion'];
	$tsk[5] = $task['start_time'];
	$tsk[6] = $task['finish_time'];
	$tsk[7] = $task['ping_time'];
	
	// fix task status
	$st = $task['status'];
	$stt = get_timestamp($task['start_time']);
	$fnt = get_timestamp($task['finish_time']);
	$pnt = get_timestamp($task['ping_time']);
	$cut = time();
	$cmp = $task['completion'];
	// 0 - IDLE
	// 1 - RUNNING
	// 2 - PAUSED
	// -----------
	// 3 - FINISHED
	// 4 - STOPPED
	// 5 - HANGS
	if ($st == 0 && $fnt > 0 && $cmp == 100) {
		// completed
		$st = 3;
	}
	if ($st < 3 && $fnt > 0 && $cmp < 100) {
		// stopped
		$st = 4;
	}
	if ($st < 3 && $fnt == 0 && ($pnt + $timeout) < $cut) {
		// hangs
		$st = 5;
	}
	if ($st < 0 || $st > $tsk_last) {
		$st = $tsk_last;
	}
	$tsk[3] = $st;
	$tsk[8] = $tsk_status[$st];
	
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

/// functions
function get_timestamp($src)
{
	$dat = strtotime($src);
	if (!$dat) {
		$dat = 0;
	}
	return $dat;
}
?>
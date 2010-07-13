<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('settings/modules')) {
    PrintNoAccess();
    exit(0);
}

$db = GetDbConnection();

$acl = GetACL('settings/modules/scanners');
$scModules = array();
$scanACL = 0;
if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
    $scanACL = 1;
}
$dbg = "";
$table = GetTableName("modules");
// class=0 - the scanners
$scanners = $db->query("SELECT * FROM $table WHERE class=0")->fetchAll(PDO::FETCH_COLUMN);
foreach($scanners as $scan) {
	$instance = array();
	/*$dbg .= $scan . "\n";
	// real scanner record, not the SysInfo or commands queue
	$system = substr($scan, 20);
	$dbg .= $system . "\n";
	$inst = "";
	$pos = strpos($system, ":");
	if ($pos === false) {
		$inst = "1";
	}
	else {
		$inst = substr($system, $pos+1);
		$system = substr($system, 0, $pos);
	}
	$dbg .= $inst . "\n";
	$dbg .= $system . "\n";
	$info = $db->lrange($scan, 0, -1);*/
	$instance[] = $scan['id'] . ":" . $scan['instance'];
	$instance[] = $scan['version'];
	$instance[] = $scan['ipaddr'];
	$instance[] = $scan['name'];
	$instance[] = $scan['onrun'];
	$instance[] = $scan['status'];
	
	$scModules[] = $instance;
}
$scanNum = count($scModules);
$dbg .= print_r($scModules, true);
$dbg .= "\nCount: " . $scanNum;

// class=1 - the reporters
$reporters = $db->query("SELECT * FROM $table WHERE class=1")->fetchAll(PDO::FETCH_COLUMN);
foreach($reporters as $rep) {
	$instance = array();
	$instance[] = $rep['id'] . ":" . $rep['instance'];
	$instance[] = $rep['version'];
	$instance[] = $rep['ipaddr'];
	$instance[] = $rep['name'];
	$instance[] = $rep['onrun'];
	$instance[] = $rep['status'];
	
	$repModules[] = $instance;
}
$repACL = 0;
$repNum = count($repModules);

$smarty->assign('UserName', $gUser['login']);
$smarty->assign('debug', $dbg);

$smarty->assign('ScanACL', $scanACL);
$smarty->assign('ScanNum', $scanNum);
$smarty->assign('ScanModules', $scModules);

$smarty->assign('ReportACL', $repACL);
$smarty->assign('ReportNum', $repNum);
$smarty->assign('ReportModules', $repModules);
DisplayThemePage('settings/modules.html');
?>

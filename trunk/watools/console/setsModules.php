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

$acl = GetACL('modules/scanners');
$scModules = array();
$repModules = array();
$connModules = array();
$scanACL = 0;
if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
    $scanACL = 1;
}
$dbg = "";
$table = GetTableName("modules");
$tinfo = GetTableName("modules_info");
// class=0 - the scanners
$tm = time();
$db->exec("DELETE FROM $table WHERE stamp+timeout < $tm");
$db->exec("DELETE FROM $tinfo WHERE stamp+timeout < $tm");
$scanners = $db->query("SELECT * FROM $table WHERE class=0");
if ($scanners) {
	$scanners = $scanners->fetchAll();
}
foreach($scanners as $scan) {
	$instance = array();
	$instance[] = $scan['id']  . ":" .  $scan['instance'];
	$instance[] = $scan['version'];
	$instance[] = $scan['ipaddr'];
	$instance[] = $scan['name'];
	$instance[] = $scan['onrun'];
	$instance[] = $scan['status'];
	
	$scModules[] = $instance;
}
$scanNum = count($scModules);
$dbg .= "\nCount: " . $scanNum;

// class=1 - the reporters
$reporters = $db->query("SELECT * FROM $table WHERE class=1");
if ($reporters) {
	$reporters = $reporters->fetchAll();
}
foreach($reporters as $rep) {
	$instance = array();
	$instance[] = $rep['id']  . ":" .  $rep['instance'];
	$instance[] = $rep['version'];
	$instance[] = $rep['ipaddr'];
	$instance[] = $rep['name'];
	$instance[] = $rep['onrun'];
	$instance[] = $rep['status'];
	
	$repModules[] = $instance;
}
$repACL = 0;
$acl = GetACL('modules/reporters');
if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
    $repACL = 1;
}
$repNum = count($repModules);

// class=2 - the connectors
$connectors = $db->query("SELECT * FROM $table WHERE class=2");
if ($connectors) {
	$connectors = $connectors->fetchAll();
}
foreach($connectors as $conn) {
	$instance = array();
	$instance[] = $conn['id']  . ":" .  $conn['instance'];
	$instance[] = $conn['version'];
	$instance[] = $conn['ipaddr'];
	$instance[] = $conn['name'];
	$instance[] = $conn['onrun'];
	$instance[] = $conn['status'];
	
	$connModules[] = $instance;
}
$connACL = 0;
$acl = GetACL('modules/connectors');
if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
    $connACL = 1;
}
$connNum = count($connModules);

$smarty->assign('UserName', $gUser['login']);

$smarty->assign('ScanACL', $scanACL);
$smarty->assign('ScanNum', $scanNum);
$smarty->assign('ScanModules', $scModules);

$smarty->assign('ReportACL', $repACL);
$smarty->assign('ReportNum', $repNum);
$smarty->assign('ReportModules', $repModules);

$smarty->assign('ConnectACL', $connACL);
$smarty->assign('ConnectNum', $connNum);
$smarty->assign('ConnectModules', $connModules);

DisplayThemePage('settings/modules.html');
?>

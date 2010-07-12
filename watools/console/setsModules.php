<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('settings/modules')) {
    PrintNoAccess();
    exit(0);
}

$r = GetDbConnection();
$scanners = $r->keys("ScanModule:Instance:*");

$acl = GetACL('settings/modules/scanners');
$scModules = array();
$scanACL = 0;
if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
    $scanACL = 1;
}
$dbg = "";
foreach($scanners as $scan) {
    if (strlen($scan) > 20) {
        $instance = array();
        $dbg .= $scan . "\n";
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
        $info = $r->lrange($scan, 0, -1);
        $instance[] = $system . ":" . $inst;
        $instance[] = $info[5];
        $instance[] = $info[0];
        $instance[] = $info[1];
        $instance[] = $info[3];
        $instance[] = $info[4];
        
        $scModules[] = $instance;
    }
}
$scanNum = count($scModules);
$dbg .= print_r($scModules, true);
$dbg .= "\nCount: " . $scanNum;

$rpModules = array();
$rpACL = 0;
$rpNum = count($rpModules);

$smarty->assign('UserName', $gUser[0]);
$smarty->assign('debug', $dbg);

$smarty->assign('ScanACL', $scanACL);
$smarty->assign('ScanNum', $scanNum);
$smarty->assign('ScanModules', $scModules);

$smarty->assign('ReportACL', $rpACL);
$smarty->assign('ReportNum', $rpNum);
DisplayThemePage('settings/modules.html');
?>

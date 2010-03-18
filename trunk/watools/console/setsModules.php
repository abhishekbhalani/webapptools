<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('settings/modules')) {
    PrintNoAccess();
    exit(0);
}

$r = GetRedisConnection();
$scanners = $r->keys("ScanModule:*");

$acl = GetACL('settings/modules/scanners');
$scModules = array();
$scanACL = 0;
if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
    $scanACL = 1;
}
$dbg = "";
foreach($scanners as $scan) {
    $instance = array();
    if (substr_compare($scan, "ScanModule:Queue:", 0, 17) != 0 &&
        substr_compare($scan, "ScanModule:SysInfo:", 0, 19) != 0) {
        $dbg .= $scan . "\n";
        // real scanner record, not the SysInfo or commands queue
        $system = substr($scan, 11);
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
        $instance[] = $system;
        $instance[] = $inst;
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

<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

function PrintError($message) {
    $smarty->assign('messageIcon', "warning.png");
    $smarty->assign('messageText', $message);
    DisplayThemePage('modules/modError.html');
    exit(0);
}

function PrintMessage($message) {
    echo $message;
    exit(0);
}

// todo check ACL for access
if (!CheckACL('usermanagament')) {
    PrintNoAccess();
    exit(0);
}

$r = GetRedisConnection();
if (is_null($r)) {
    PrintError('Database access error!');
}

$action = $_POST['action'];
$module = $_POST['uuid'];
$inst   = "";
$class  = $_POST['class'];

$pos = strpos($module, ":");
if ($pos === false) {
    $inst = "1";
}
else {
    $inst = substr($module, $pos+1);
    $module = substr($module, 0, $pos);
}

if ($action == "info") {
    if ($class == 'scanner') {
        $acl = GetACL('modules/scanners');
        $scanACL = 0;
        if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
            $scanACL = 1;
        }

        if ($r->exists("ScanModule:Instance:$module:$inst")) {
            $instInfo = $r->lrange("ScanModule:Instance:$module:$inst", 0, -1);
            $sysInfo = array();
            $cmdQueue = array();
            if ($r->exists("ScanModule:SysInfo:$module")) {
                $sysInfo = $r->lrange("ScanModule:SysInfo:$module", 0, -1);
            }
            else {
                for ($i = 0; $i < 6; $i++) {
                    $sysInfo[] = gettext('<unknown>');
                }
            }
            if ($r->exists("ScanModule:Queue:$module:$inst")) {
                $cmdQueue = $r->lrange("ScanModule:Queue:$module:$inst", 0, -1);
            }
            // make report
            $smarty->assign('identity', htmlentities("$module:$inst", ENT_QUOTES));
            $smarty->assign('address', htmlentities($instInfo[0], ENT_QUOTES));
            $smarty->assign('scanname', htmlentities($instInfo[1], ENT_QUOTES));
            $smarty->assign('scanvers', htmlentities($instInfo[5], ENT_QUOTES));
            $smarty->assign('currtasks', htmlentities($instInfo[3], ENT_QUOTES));
            $smarty->assign('osname', htmlentities($sysInfo[0], ENT_QUOTES));
            $smarty->assign('memory', htmlentities($sysInfo[1], ENT_QUOTES));
            $smarty->assign('disks', htmlentities($sysInfo[2], ENT_QUOTES));
            $smarty->assign('cpu', htmlentities($sysInfo[3], ENT_QUOTES));
            $smarty->assign('maxtasks', htmlentities($sysInfo[4], ENT_QUOTES));
            $smarty->assign('queuelen', htmlentities(count($cmdQueue), ENT_QUOTES));
            if ($scanACL) {
                $smarty->assign('showqueue', 'true');
                $smarty->assign('queue', $cmdQueue);
            }
            else {
                $smarty->assign('showqueue', 'false');
                $smarty->assign('queue', "");
            }
            DisplayThemePage('modules/modScanInfo.html');
        }
        else {
            PrintError("Can't find selected instance ($module:$inst)!");
        }
    }
    else if ($class == 'reporter') {
        // no information yet
    }
    else {
        PrintError("Invalid request!");
    }
    exit (0);
}
else if ($action == "stop") {
    if ($class == 'scanner') {
        $acl = GetACL('modules/scanners');
        $scanACL = 0;
        if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
            $scanACL = 1;
        }
        else {
            PrintError("Access denied for user $gUser[0]!");
        }
        // send commnad to scanner
        $r->rpush("ScanModule:Queue:$module:$inst", "EXIT");
        PrintMessage("Exit request was send to scanner $module:$inst!");
    }
    else if ($class == 'reporter') {
        // no information yet
    }
    else {
        PrintError("Invalid request!");
    }
}
else if ($action == "restart") {
    if ($class == 'scanner') {
        $acl = GetACL('modules/scanners');
        $scanACL = 0;
        if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
            $scanACL = 1;
        }
        else {
            PrintError("Access denied for user $gUser[0]!");
        }
        // send commnad to scanner
        $r->rpush("ScanModule:Queue:$module:$inst", "RESTART");
        PrintMessage("Restart request was send to scanner $module:$inst!");
    }
    else if ($class == 'reporter') {
        // no information yet
    }
    else {
        PrintError("Invalid request!");
    }
}
else {
    PrintError("Invalid request!");
}

?>

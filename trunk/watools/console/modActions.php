<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('usermanagament')) {
    echo gettext('Access denied');
    exit(0);
}

$r = GetRedisConnection();
if (is_null($r)) {
    echo gettext('Database access error!');
    exit(0);
}

$action = $_POST['action'];
$module = $_POST['uuid'];
$inst   = $_POST['instance'];
$class  = $_POST['class'];

if ($action == "info") {
    if ($class == 'scanner') {
        $acl = GetACL('modules/scanners');
        $scanACL = 0;
        if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
            $scanACL = 1;
        }

        if ($r->exists("ScanModule:$module:$inst")) {
            $instInfo = $r->lrange("ScanModule:$module:$inst", 0, -1);
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
            $smarty->assign('messageIcon', "warning.png");
            $smarty->assign('messageText', "Can't find selected instance ($module:$inst)!");
            DisplayThemePage('modules/modError.html');
        }
    }
    else if ($class == 'reporter') {
        // no information yet
    }
    else {
        echo gettext('Invalid request!');
    }
    exit (0);
}
else if ($action == "stop") {
    
}
else {
    echo gettext('Invalid request!');
}

?>

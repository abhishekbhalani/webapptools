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

$db = GetDbConnection();
if (is_null($db)) {
    PrintError('Database access error!');
}

$action = $_POST['action'];
$module = $_POST['uuid'];
$inst   = "";
$class  = $_POST['class'];

$table = GetTableName("modules");
$tinfo = GetTableName("modules_info");
$tcmds = GetTableName("module_cmds");
$tm = time();
$db->exec("DELETE FROM $table WHERE class=0 AND stamp+timeout < $tm");
$db->exec("DELETE FROM $tinfo WHERE stamp+timeout < $tm");

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

		$query = $db->prepare("SELECT * FROM $table, $tinfo WHERE $table.class=0 AND $table.instance=? AND $table.id=? AND $tinfo.module_id=$table.instance");
		if ($query) {
			$sysInfo = array();

			$query->bindParam(1, $module);
			$query->bindParam(2, $inst);
			$scanners = $query->execute();
			if ($scanners) {
				$sysInfo = $query->fetchAll();
				$sysInfo = $sysInfo[0];
			} // has results
			foreach ($sysInfo as $key => $val) {
				if (is_null($val)) {
					$sysInfo[$key] = gettext('<unknown>');
				}
			} // fix results
			$cmdQueue = array();
			$qcmd = $db->prepare("SELECT * FROM $tcmds WHERE module_id=? ORDER BY timestamp ASC");
			if ($qcmd) {
				$inf = "$module:$inst";
				$qcmd->bindParam(1, $inf);
				$cmds = $qcmd->execute();
				if ($cmds) {
					$cmds = $qcmd->fetchAll();
					foreach($cmds as $cm) {
						$inf = array();
						$inf[0] = date(DATE_RFC822, $cm['timestamp']);
						$vals = preg_split("/[\s]+/", $cm['cmd']);
						$inf[1] = $vals[0];
						$cmdQueue[] = $inf;
					}
				}
			}
			$smarty->assign('identity', htmlentities("$module:$inst", ENT_QUOTES));
			$smarty->assign('address', htmlentities($sysInfo['ipaddr'], ENT_QUOTES));
			$smarty->assign('scanname', htmlentities($sysInfo['name'], ENT_QUOTES));
			$smarty->assign('scanvers', htmlentities($sysInfo['version'], ENT_QUOTES));
			$smarty->assign('currtasks', htmlentities($sysInfo['onrun'], ENT_QUOTES));
			$smarty->assign('osname', htmlentities($sysInfo['osname'], ENT_QUOTES));
			$smarty->assign('memory', htmlentities($sysInfo['mem_size'], ENT_QUOTES));
			$smarty->assign('disks', htmlentities($sysInfo['disk_size'], ENT_QUOTES));
			$smarty->assign('cpu', htmlentities($sysInfo['cpu_usage'], ENT_QUOTES));
			$smarty->assign('maxtasks', htmlentities($sysInfo['max_tasks'], ENT_QUOTES));
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
		} // query prepared
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
	$moduleACL = 0;
	$mdClass = "";
    if ($class == 'scanner') {
		$mdClass = gettext("scanner");
        $acl = GetACL('modules/scanners');
        if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
            $moduleACL = 1;
        }
        else {
            PrintError("Access denied for user $gUser[0]!");
        }
	}
	else if ($class == 'reporter') {
		$mdClass = gettext("report generator");
        $acl = GetACL('modules/reporters');
        if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
            $moduleACL = 1;
        }
        else {
            PrintError("Access denied for user $gUser[0]!");
        }
    }
    else {
        PrintError("Invalid request!");
    }
	if ($moduleACL == 1) {
        // send commnad to scanner
		$query = $db->prepare("INSERT INTO $tcmds (module_id, timestamp , cmd) VALUES(?,?,'EXIT')");
        if($query) {
			$inf = "$module:$inst";
			$query->bindParam(1, $inf);
			$query->bindParam(2, $tm);
			$query->execute();
			$msg = gettext("Stop request was send to");
			$msg .= " "; 
			$msg .= $mdClass . " $inf!";
			PrintMessage($msg);
		}
		else {
			PrintDbError($db);
		}
    }
}
else if ($action == "restart") {
	$moduleACL = 0;
	$mdClass = "";
    if ($class == 'scanner') {
		$mdClass = gettext("scanner");
        $acl = GetACL('modules/scanners');
        if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
            $moduleACL = 1;
        }
        else {
            PrintError("Access denied for user $gUser[0]!");
        }
	}
	else if ($class == 'reporter') {
		$mdClass = gettext("report generator");
        $acl = GetACL('modules/reporters');
        if (in_array('system', $acl) || in_array('write', $acl) || in_array('execute', $acl)) {
            $moduleACL = 1;
        }
        else {
            PrintError("Access denied for user $gUser[0]!");
        }
    }
    else {
        PrintError("Invalid request!");
    }
	if ($moduleACL == 1) {
        // send commnad to scanner
		$query = $db->prepare("INSERT INTO $tcmds (module_id, timestamp , cmd) VALUES(?,?,'RESTART')");
        if($query) {
			$inf = "$module:$inst";
			$query->bindParam(1, $inf);
			$query->bindParam(2, $tm);
			$query->execute();
			$msg = gettext("Restart request was send to");
			$msg .= " "; 
			$msg .= $mdClass . " $inf!";
			PrintMessage($msg);
		}
		else {
			PrintDbError($db);
		}
    }
}
else {
    PrintError("Invalid request!");
}

?>

<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');


// todo check ACL for access
if (!CheckACL('usermanagament')) {
    echo gettext('Access denied!');
    exit(0);
}

$db = GetDbConnection();
$table = GetTableName("profile");
$tui = GetTableName("profile_ui");
if (is_null($db)) {
    echo gettext('Database access error!');
    exit(0);
}
$action = $_POST['action'];
$msg = gettext("Invalid request!");
if ($action == "load") {
	$msg = "";
    $pid = $_POST['prof_id'];
    $pid *= 2;
    $pid /= 2;	
	$q = $db->query("SELECT * FROM $table WHERE profile_id=$pid");
	$profile = "";
	$plg_list = "";
	if ($q) {
		$prof_values = $q->fetchAll(PDO::FETCH_ASSOC);
		// generate javascript to set values
		$prof_vars = "function SetValues() {\n";
		foreach ($prof_values as $val) {
			$objName = $val['name']; //preg_replace("/\\//", "_", $val['name']);
			$objVal = urlencode($val['value']);
			$prof_vars .= "setControlValue('" . $objName . "', '" . $objVal . "');\n";
			if ($val['name'] == 'plugin_list') {
				$plg_list = $val['value'];
			}
			if ($val['name'] == 'profile_name') {
				$profile = $val['value'];
			}
		}
		$prof_vars .= "}\n";
		// get plugins UI
		$ui_list = array();
		$loc = $gSession['lang'];
		if ($loc != 'en') {
			$pq = $db->query("SELECT plugin_id,plugin_name,ui_settings,ui_icon FROM $tui WHERE locale='$loc'");
			$prof_ui = $pq->fetchAll();
			foreach($prof_ui as $ui) {
				$pl_id = $ui['plugin_id'];
				$ui_list[$pl_id] = $ui;
			}
		}
		// fix missing data from english (default) locale
		$pq = $db->query("SELECT plugin_id,plugin_name,ui_settings,ui_icon FROM $tui WHERE locale='en'");
		if ($pq) {
			$prof_en = $pq->fetchAll();
			foreach ($prof_en as $en) {
				$pl_id = $en['plugin_id'];
				if ($ui_list[$pl_id]) {
					if ($ui_list[$pl_id]['plugin_name'] == "") {
						$ui_list[$pl_id]['plugin_name'] = $en['plugin_name'];
					}
					if ($ui_list[$pl_id]['ui_settings'] == "") {
						$ui_list[$pl_id]['ui_settings'] = $en['ui_settings'];
					}
					if ($ui_list[$pl_id]['ui_icon'] == "") {
						$ui_list[$pl_id]['ui_icon'] = $en['ui_icon'];
					}
				}
				else {
					$ui_list[$pl_id] = $en;
				}
				if ($ui_list[$pl_id]['ui_icon'] == "") {
					//save icon
				}
			}
		}
		// make plugin list
		$active_plg = explode(';', $plg_list);
		$plglist = array();
		foreach($ui_list as $ui) {
			if ($ui != "") {
				$plg = array();
				$plg[0] = $ui['plugin_id'];
				$plg[1] = $ui['plugin_name'];
				if (in_array($plg[0], $active_plg)) {
					$plg[2] = true;
				}
				else {
					$plg[2] = false;
				}
				// make UI
				$xslt = GetThemeFile('ui_transform.xslt');
				if ($xslt == "") {
					$ui_text = gettext("Can't build UI for plugin!");
				}
				else {
					$ui_src = $ui['ui_settings'];
					if ($ui_src == "") {
						$ui_text = gettext("Plugin doesn't provide settings.");
					}
					else {
						$XML = new DOMDocument(); 
						$XML->loadXML( $ui_src ); 
						# START XSLT 
						$xsl_t = new XSLTProcessor(); 
						$XSL = new DOMDocument(); 
						$XSL->load( $xslt , LIBXML_NOCDATA); 
						$xsl_t->importStylesheet( $XSL ); 
						#PRINT 
						$ui_text = $xsl_t->transformToXML( $XML );
					}
					if ($ui_text == "") {
						$ui_text = gettext("Generation failed.");
					}
				}
				// save
				$plg[4] = $ui_text;
				$plglist[] = $plg;
			}
		}

		$prof_vars .="SetValues();";
		$smarty->assign('profile_id', $pid);
		$smarty->assign('profile_name', $profile);
		$smarty->assign('pluginsList', $plglist);
		$smarty->assign('settingsScript', $prof_vars);
		DisplayThemePage('profileEditor.html');
		//print "<pre>";
		//print_r($prof_vars);
		//print "</pre>";
	}
	else {
		$e = $db->errorInfo();
		$smarty->assign('messageTitle', gettext("Can't display profile editor"));
		$smarty->assign('messageText', gettext('Database connection error! ') . "<br/>" . $e[0] . ": " . $e[2]);
		$smarty->assign('messageIcon', 'warning.png');
		$smarty->assign('messageButton', 'OK');
		DisplayThemePage('messageBox.html');
	}
}
else if ($action == "add") {
	$name = $_POST['prof_name'];
	if ($name && $name != "") {
		$msg = "OK";
		$q = $db->exec("BEGIN EXCLUSIVE TRANSACTION");
		if ($q) {
			$q = $db->prepare("SELECT profile_id FROM $table WHERE name=='profile_name' AND value==?");
			if ($q) {
				$q->bindParam(1, $name);
				$q->execute();
				$id = $q->fetchAll();
				if (is_null($id) || is_null($id[0])) {
					$id = GetSingleRow($db, "SELECT max(profile_id) FROM $table");
					if ($id) {
						$id = $id[0] + 1;
						$q = $db->prepare("INSERT INTO $table (profile_id,name,type,value) VALUES($id,'profile_name',4,?)");
						if ($q) {
							$q->bindParam(1, $name);
							$q->execute();
						}
					}
				}
				else {
					$msg = gettext("Profile with same name already exists: ") . $name;
				}
			}
			$db->exec("COMMIT TRANSACTION");
		}
		else {
			$msg = "";
			PrintDbError($db);
		}
	}
}
else if ($action == "del") {
    $pid = $_POST['prof_id'];
    $pid *= 2;
    $pid /= 2;
	$q = $db->exec("DELETE FROM $table WHERE profile_id=$pid");
	if ($q) {
		$msg = "OK";
	}
	else {
		$msg = "";
		PrintDbError($db);
	}
}
else if ($action == "clone") {
	$name = $_POST['prof_name'];
	if ($name && $name != "") {
		$msg = "OK";
		$q = $db->exec("BEGIN EXCLUSIVE TRANSACTION");
		if ($q) {
			$q = $db->prepare("SELECT profile_id FROM $table WHERE name=='profile_name' AND value==?");
			if ($q) {
				$q->bindParam(1, $name);
				$q->execute();
				$id = $q->fetchAll();
				if (is_null($id) || is_null($id[0])) {
					$id = GetSingleRow($db, "SELECT max(profile_id) FROM $table");
					if ($id) {
						$id = $id[0] + 1;
						$q = $db->prepare("INSERT INTO $table (profile_id,name,type,value) VALUES($id,'profile_name',4,?)");
						if ($q) {
							$q->bindParam(1, $name);
							$q->execute();
						}
						// copy profile values
						$pid = $_POST['prof_id'];
						$pid *= 2;
						$pid /= 2;
						$q = $db->query("SELECT * FROM $table WHERE profile_id=$pid AND name!='profile_name'");
						if ($q) {
							$values = $q->fetchAll();
							$q = $db->prepare("INSERT INTO $table (profile_id,name,type,value) VALUES($id,?,?,?)");
							if ($q) {
								foreach ($values as $val) {
									$q->bindParam(1, $val['name']);
									$q->bindParam(2, $val['type']);
									$q->bindParam(3, $val['value']);
									$q->execute();
								}
							}
						}
					}
				}
				else {
					$msg = gettext("Profile with same name already exists: ") . $name;
				}
			}
			$db->exec("COMMIT TRANSACTION");
		}
		else {
			$msg = "";
			PrintDbError($db);
		}
	}
}
else if ($action == "save") {
    $pid = $_POST['prof_id'];
    $pid *= 2;
    $pid /= 2;
	
	$dtp = $_POST['data_types'];
	$dtp = explode(';', $dtp);
	$types = array();
	foreach($dtp as $dt) {
		$dt = explode('=', $dt);
		$types[$dt[0]] = $dt[1];
	}
	$upd = $db->prepare("REPLACE INTO $table (profile_id,name,value,type) VALUES($pid,?,?,?)");
	if ($upd) {
		foreach($_POST as $key => $value) {
			if ($key != 'action' && $key != 'prof_id' && $key != 'data_types') {
				// update value
				$tp = $types[$key];
				if (!$tp || $tp == '') {
					$tp = 4;
				}
				$upd->bindParam(1, $key);
				$upd->bindParam(2, $value);
				$upd->bindParam(3, $tp);
				$r = $upd->execute();
				if (! $r) {
					$msg = "";
					PrintDbError($upd);
				}
			}
		}
	}
	else {
		$msg = "";
		PrintDbError($db);
	}
	$msg = "OK";
}
echo $msg;
?>
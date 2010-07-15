<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');


// todo check ACL for access
if (!CheckACL('usermanagament')) {
    echo gettext('Access denied');
    exit(0);
}

$db = GetDbConnection();
if (is_null($db)) {
    echo gettext('Database access error!');
    exit(0);
}

$action = $_POST['object'];
if ($action == 'themeset') {
    $msg = "OK";
    $thID = $_POST['theme'];
    $lnID = $_POST['lang'];
    $thID *= 2;
    $thID /= 2;
    
    if ($thID >= 0) {
		$table = GetTableName("themes");
		$q = GetSingleRow($db, "SELECT id, name FROM $table WHERE id=$thID");
        if ($q[0] == $thID) {
			$table = GetTableName("_internals_");
			$s = $db->prepare("UPDATE $table SET value=? WHERE name='DefaultTheme'");
			if ($s) {
				$s->bindParam(1, $q[1]);
				$s->execute();
				$msg = $thID . "," . $q[1];
			}
			else {
				PrintDbError($db);
			}
            //! TODO: check language
            $lnID = strtolower($lnID);
            if ($lnID == 'en' || $lnID == 'ru') {
				$s = $db->prepare("UPDATE $table SET value=? WHERE name='DefaultLang'");
				if ($s) {
					$s->bindParam(1, $lnID);
					$s->execute();
					$msg .= ", " . $lnID;
				}
				else {
					PrintDbError($db);
				}
            }
            else {
                $msg = gettext("Unknown language!");
            }
        }
        else {
            $msg = gettext("Theme doesn't exist!");
        }
    }
    else {
        $msg = gettext('Invalid request!');
    }
    echo $msg;
    exit(0);
}
else if ($action == 'themeusr') {
    $msg = "OK";
    $thID = $_POST['theme'];
    $lnID = $_POST['lang'];
    
    if ($thID != "") {
		$gSession['theme'] = $thID;
		$gSession['lang'] = $lnID;
		SaveSession();
		$table = GetTableName("users");
		$q = $db->prepare("UPDATE $table SET theme=?, lang=? WHERE id=?");
		if ($q) {
			$q->bindParam(1, $thID);
			$q->bindParam(2, $lnID);
			$q->bindParam(3, $gUser['id']);
			$q->execute();
		}
		else {
			PrintDbError($db);
		}
    }
    else {
        $msg = gettext('Invalid request!');
    }
    echo $msg;
    exit(0);
}
else if ($action == 'themeinst') {
    $msg = gettext('Invalid request!');
    $tname = $_POST['theme'];
    
    $handle = @fopen($tname . '/theme.txt', "r");
    if ($handle) {
        $locstr = fgets($handle);
        $locstr = rtrim($locstr);
        $data = "";
        while (!feof($handle)) {
            $data .= fgets($handle);
        }
        fclose($handle);
        $locales = explode('|', $locstr);
        if ($db->exists("ThemeName:$tname") == 0) {
            $tid = $db->incr('Global:ThemeID');
            SetValue($db, "ThemeName:$tname", $tid);
            $db->delete("Theme:$tid");
            $db->rpush("Theme:$tid", $tname);
            $db->rpush("Theme:$tid", $data);
            $db->delete("Theme:$tid:Locales");
            foreach ($locales as $loc) {
                $db->sadd("Theme:$tid:Locales", $loc);
            }
            $msg = "OK";
        }
        else {
            $msg = gettext('Theme already installed!');
        }
    } // file handle opened
    else {
        $msg = gettext('Can\'t read theme description!');
    }
    echo $msg;
    exit(0);
}
else {
    echo gettext('Invalid request!');
}
?>

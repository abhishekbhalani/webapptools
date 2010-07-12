<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');


// todo check ACL for access
if (!CheckACL('usermanagament')) {
    echo gettext('Access denied');
    exit(0);
}

$r = GetDbConnection();
if (is_null($r)) {
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
    
    if ($thID > 0) {
        if ($r->exists("Theme:$thID") == 1) {
            $th = $r->lrange("Theme:$thID", 0, 1);
            $r->set("Global:DefaultTheme", $th[0]);
            //! TODO: check language
            $lnID = strtolower($lnID);
            if ($lnID == 'en' || $lnID == 'ru') {
                $r->set("Global:DefaultLang", $lnID);
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
		$q = $r->prepare("UPDATE $table SET theme=?, lang=? WHERE id=?");
		if ($q) {
			$q->bindParam(1, $thID);
			$q->bindParam(2, $lnID);
			$q->bindParam(3, $gUser['id']);
			$q->execute();
		}
		else {
			PrintDbError($r);
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
        if ($r->exists("ThemeName:$tname") == 0) {
            $tid = $r->incr('Global:ThemeID');
            SetValue($r, "ThemeName:$tname", $tid);
            $r->delete("Theme:$tid");
            $r->rpush("Theme:$tid", $tname);
            $r->rpush("Theme:$tid", $data);
            $r->delete("Theme:$tid:Locales");
            foreach ($locales as $loc) {
                $r->sadd("Theme:$tid:Locales", $loc);
            }
            $msg = "OK";
        }
        else {
            $msg = gettext('Theme already onstalled!');
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

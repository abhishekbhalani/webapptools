<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('settings/system')) {
    PrintNoAccess();
    exit(0);
}

$db = GetDbConnection();
$thIDs = array();
$thNms = array();
$thDsc = array();
$thLng = array();
$thDef = -1;
if (!is_null($db)) {
	$table = GetTableName("_internals_");
    $thDef = GetSingleRow($db, "SELECT value FROM $table WHERE name='DefaultTheme'");
	$thDef = $thDef[0];
	$table = GetTableName("themes");
	$q = $db->query("SELECT id,name FROM $table");
	foreach ($q as $k) {
		$thIDs[] = $k[0];
		$thNms[] = $k[1];
		if ($thDef == $k[1]) {
			$thDef = $k[0];
		}
		$handle = @fopen($k[1] . '/theme.txt', "r");
		$data = gettext("No description provided");
		$lng = array();
		if ($handle) {
			$data = fgets($handle);
			$data = rtrim($data);
			$locales = explode('|', $data);
			foreach($locales as $l) {
				$ln = explode(';', $l, 2);
				$lng[] = $ln;
			}
			$data = "";
			while (!feof($handle)) {
				$data .= fgets($handle);
			}
			fclose($handle);
		}
		$thDsc[] = $data;
		$thLng[] = $lng;
	}
}
// search for not installed themes
$dirs = glob('*', GLOB_ONLYDIR);
if ($dirs) {
	$table = GetTableName("themes");
    foreach ($dirs as $dir) {
        $files = glob($dir . '/theme.txt');
        if ($files) {
            $tname = $dir;
			$q = $db->prepare("SELECT id FROM $table WHERE name=?");
			if ($q) {
				$q->bindParam(2, $tname);
				$q->execute();
				$d = $q->fetchAll(PDO::FETCH_COLUMN);
				if (! is_null($d[0]) ) {
					$handle = @fopen($files[0], "r");
					if ($handle) {
						$locstr = fgets($handle);
						$locstr = rtrim($locstr);
						$locales = explode('|', $locstr);
						$lng = array();
						foreach($locales as $l) {
							$ln = explode(';', $l, 2);
							$lng[] = $ln;
						}
						$data = "";
						while (!feof($handle)) {
							$data .= fgets($handle);
						}
						fclose($handle);
						$thIDs[] = -1;
						$thNms[] = $tname;
						$thDsc[] = $data;
						$thLng[] = $lng;
					} // file handle opened
				} // theme not installed
			} // query prepared
        } // any files found
    } // foraech dirs
} // any dirs found

$lgIDs = array();
$lgNms = array();
$lgDef = 'en';

$lgIDs[] = 'en';
$lgNms[] = 'English';
$lgIDs[] = 'ru';
$lgNms[] = 'Русский';

$smarty->assign('themes', $thIDs);
$smarty->assign('thNames', $thNms);
$smarty->assign('thDesc', $thDsc);
$smarty->assign('thLangs', $thLng);
$smarty->assign('defTheme', $thDef);
$smarty->assign('langs', $lgIDs);
$smarty->assign('lnNames', $lgNms);
$smarty->assign('defLang', $lgDef);
DisplayThemePage('settings/system.html');
?>
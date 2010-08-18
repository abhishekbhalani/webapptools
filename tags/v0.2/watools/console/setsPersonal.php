<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('settings/personal')) {
    PrintNoAccess();
    exit(0);
}

$r = GetDbConnection();
$thIDs = array();
$thNms = array();
$thDsc = array();
$thLng = array();
$thDef = -1;
if (!is_null($r)) {
    $thDef = $gUser['theme'];
    //$thDef = $r->get("ThemeName:$thDef");
	$table = GetTableName("themes");
	$keys = $r->query("SELECT name FROM $table")->fetchAll(PDO::FETCH_COLUMN);
    foreach ($keys as $k) {
        //$u = $r->get($k);
        //$k = substr($k, 10);
        $thIDs[] = $k;
        $thNms[] = $k;
        $handle = @fopen($k . '/theme.txt', "r");
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

$lgIDs = array();
$lgNms = array();
$lgDef = $gUser[4];

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

$smarty->assign('UserName', $gUser['login']);
$smarty->assign('userName', $gUser['desc']);
DisplayThemePage('settings/personal.html');
?>
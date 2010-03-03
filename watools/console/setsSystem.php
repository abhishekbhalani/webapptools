<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('settings/system')) {
    PrintNoAccess();
    exit(0);
}

$r = GetRedisConnection();
$thIDs = array();
$thNms = array();
$thDsc = array();
$thLng = array();
$thDef = -1;
if (!is_null($r)) {
    $thDef = $r->get("Global:DefaultTheme");
    $thDef = $r->get("ThemeName:$thDef");
    $keys = $r->keys("ThemeName:*");
    foreach ($keys as $k) {
        $u = $r->get($k);
        $k = substr($k, 10);
        $thIDs[] = $u;
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
// search for not installed themes
$dirs = glob('*', GLOB_ONLYDIR);
if ($dirs) {
    foreach ($dirs as $dir) {
        $files = glob($dir . '/theme.txt');
        if ($files) {
            $tname = $dir;
            if ($r->exists("ThemeName:$tname") == 0) {
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
                    $thIDs[] = 0;
                    $thNms[] = $tname;
                    $thDsc[] = $data;
                    $thLng[] = $lng;
                } // file handle opened
            } // theme not installed
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
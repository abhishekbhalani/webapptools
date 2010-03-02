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
$thDef = -1;
if (!is_null($r)) {
    $thDef = $r->get("Global:DefaultTheme");
    $keys = $r->keys("ThemeName:*");
    foreach ($keys as $k) {
        $u = $r->get($k);
        $k = substr($k, 10);
        $thIDs[] = $u;
        $thNms[] = $k;
        $thDsc[] = "Loren ipsum";
    }
}
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
$smarty->assign('defTheme', $thDef);
$smarty->assign('langs', $lgIDs);
$smarty->assign('lnNames', $lgNms);
$smarty->assign('defLang', $lgDef);
DisplayThemePage('settings/system.html');
?>
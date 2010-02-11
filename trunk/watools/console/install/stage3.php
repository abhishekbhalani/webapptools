<?
require_once('./../globals.php');
require_once('./../usermgmt.php');

// initialise template subsystem
require_once('./../smarty/Smarty.class.php');
$smarty = new Smarty();

$smarty->left_delimiter = '[{';
$smarty->right_delimiter = '}]';
$smarty->template_dir = './templates';
$smarty->compile_dir = './templates_c';
$smarty->cache_dir = './cache';
$smarty->config_dir = './../smarty/configs';

// get preffered language
if (isset($_COOKIE['WATLANG'])) {
    $lang = $_COOKIE['WATLANG'];
    // check for availible langs
}
else {
    $lang = "en";
}
// set language cookie
setcookie("WATLANG", $lang);
if ($lang == "en" || $lang == '') {
    $lang = '';
}
else {
    $lang = '.' . $lang;
}

$gRedisHost = $_POST['redis_host'];
$gRedisPort = $_POST['redis_port'];
$gRedisAuth = $_POST['redis_pass'];
$adminUser  = $_POST['user'];
$adminGroup = $_POST['group'];
$adminPswd  = $_POST['pass'];

CreateGroup($adminGroup, 'System Administrators');
CreateUser($adminUser, 'System Administrator');
AddUserToGroup($adminUser, $adminGroup);

// add default themes
$r = GetRedisConnection();
if (!is_null($r))
{
    if ($r->exists("ThemeName:theme") == 0) {
        $tid = $r->incr('Global:ThemeID');
        SetValue($r, "ThemeName:theme", $tid);
        $r->delete("Theme:$tid");
        $r->rpush("Theme:$tid", "theme", 0);
        $r->rpush("Theme:$tid", 'Simple default theme', 1);
        $r->sadd("Theme:$tid:Locales", "en;English");
        $r->sadd("Theme:$tid:Locales", "ru;Русский");
    }
    if ($r->exists("ThemeName:sandbox") == 0) {
        $tid = $r->incr('Global:ThemeID');
        SetValue($r, "ThemeName:sandbox", $tid);
        $r->delete("Theme:$tid");
        $r->rpush("Theme:$tid", "sandbox", 0);
        $r->rpush("Theme:$tid", 'SandBox theme', 1);
        $r->sadd("Theme:$tid:Locales", "en;English");
        $r->sadd("Theme:$tid:Locales", "ru;Русский");
    }
    
}

echo "<pre>";
GetUserByName($adminUser);
GetGroupByName($adminGroup);
echo "</pre>";
$smarty->assign('redisHost', $redisHost);
$smarty->assign('redisPort', $redisPort);
$smarty->assign('redisPass', $redisPass);
$smarty->display('stage3.html' . $lang);
?>
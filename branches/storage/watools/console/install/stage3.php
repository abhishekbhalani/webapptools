<?
header("HTTP/1.0 200 OK");
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
    // search for themes
    $dirs = glob('../*', GLOB_ONLYDIR);
    if ($dirs) {
        foreach ($dirs as $dir) {
            $files = glob($dir . '/theme.txt');
            if ($files) {
                $tname = substr($dir, 3);
                $handle = @fopen($files[0], "r");
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
                    }
                } // file handle opened
            } // any files found
        } // foraech dirs
    } // any dirs found
    // ... and set the defaults!
    $r->set("Global:DefaultTheme", 'sandbox');
    $r->set("Global:DefaultLang", 'en');
}

$smarty->assign('redisHost', $redisHost);
$smarty->assign('redisPort', $redisPort);
$smarty->assign('redisPass', $redisPass);
$smarty->display('stage3.html' . $lang);
?>
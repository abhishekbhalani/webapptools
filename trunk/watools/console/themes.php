<?
// put full path to Smarty.class.php
require_once('./globals.php');
require_once('./sessions.php');
require_once('./smarty/Smarty.class.php');
$smarty = new Smarty();

$r = GetRedisConnection();

if (is_null($r) || is_null($gSession)) {
    header("HTTP/1.0 302 Found");
    header("Location: /maintance.php");
    exit(0);
}


$themeName = $gSession[3];
$themeLang = $gSession[4];
$themeDir = $gBaseDir . '/' . $themeName;
// set language translation
setlocale( LC_MESSAGES, $themeLang);
putenv("LANG=".$themeLang);
bindtextdomain('watconsole', $themeDir);
textdomain('watconsole');
bind_textdomain_codeset('watconsole', 'UTF-8');
$themeLangExt = $themeLang;
if ($themeLangExt == 'en') {
    $themeLangExt = '';
}

$smarty->left_delimiter = '[{';
$smarty->right_delimiter = '}]';
$smarty->template_dir = $themeDir;
$smarty->compile_dir = $themeDir . '/templates_c';
$smarty->cache_dir = $themeDir . '/cache';
$smarty->config_dir = './smarty/configs';

?>
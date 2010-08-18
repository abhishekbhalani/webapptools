<?
require_once('./globals.php');
require_once('./smarty/Smarty.class.php');
$smarty = new Smarty();

// detect theme settings
$theme = '';
if (isset($_COOKIE['WATTHEME'])) {
    $theme = $_COOKIE['WATTHEME'];
}
if ($theme == '') {
	$theme = $gDefaultTheme;
}
$themeDir = './' . $theme;

// detect language settings
if (isset($_COOKIE['WATLANG'])) {
    $lang = $_COOKIE['WATLANG'];
}
else {
    if (isset($_SERVER['HTTP_ACCEPT_LANGUAGE'])) {
        $slangs = explode(',',$_SERVER['HTTP_ACCEPT_LANGUAGE']);
        $lang = $slang[0];
        $lang = substr($lang,0,2);
    }
}
if ($lang == '') {
    $lang = 'en';
}
$smarty->left_delimiter = '[{';
$smarty->right_delimiter = '}]';
$smarty->template_dir = $themeDir;
$smarty->compile_dir = $themeDir . '/templates_c';
$smarty->cache_dir = $themeDir . '/cache';
$smarty->config_dir = './smarty/configs';

$smarty->assign('theme', $theme);
$smarty->display('maintance.html');

?>
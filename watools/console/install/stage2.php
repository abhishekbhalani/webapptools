<?
require_once('./../globals.php');

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

$smarty->assign('db_dsn', $_POST['db_dsn']);
$smarty->assign('db_usr', $_POST['db_usr']);
$smarty->assign('db_usr', $_POST['db_usr']);
$smarty->display('stage2.html' . $lang);
?>
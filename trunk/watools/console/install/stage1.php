<?
// put full path to Smarty.class.php
require_once('./../globals.php');
require_once('./../smarty/Smarty.class.php');
$smarty = new Smarty();

$smarty->left_delimiter = '[{';
$smarty->right_delimiter = '}]';
$smarty->template_dir = './templates';
$smarty->compile_dir = './templates_c';
$smarty->cache_dir = './cache';
$smarty->config_dir = './../smarty/configs';

// get preffered language
$lang = $_POST['lang'];
if ($lang == "en" || $lang == '') {
    $lang = '';
}
else {
    $lang = '.' . $lang;
}
$smarty->display('stage1.html' . $lang);
?>
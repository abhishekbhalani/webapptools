<?
// put full path to Smarty.class.php
require_once('./globals.php');
require_once('./smarty/Smarty.class.php');
$smarty = new Smarty();

$themeName = 'sandbox';
$themeDir = $baseDir . '/' . $themeName;

$smarty->left_delimiter = '[{';
$smarty->right_delimiter = '}]';
$smarty->template_dir = $themeDir;
$smarty->compile_dir = $themeDir . '/templates_c';
$smarty->cache_dir = $themeDir . '/cache';
$smarty->config_dir = './smarty/configs';

?>
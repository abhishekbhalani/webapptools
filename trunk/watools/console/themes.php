<?
// put full path to Smarty.class.php
require_once('./globals.php');
require('./smarty/Smarty.class.php');
$smarty = new Smarty();

$themeName = 'theme';
$themeDir = $baseDir . '/' . $themeName;

$smarty->template_dir = $themeDir;
$smarty->compile_dir = $themeDir . '/templates_c';
$smarty->cache_dir = $themeDir . '/cache';
$smarty->config_dir = './smarty/configs';

?>
<? 
// put full path to Smarty.class.php
require('./smarty/Smarty.class.php');
$smarty = new Smarty();

$themeDir = '/var/www/theme';

$smarty->template_dir = $themeDir;
$smarty->compile_dir = $themeDir . '/templates_c';
$smarty->cache_dir = $themeDir . '/cache';
$smarty->config_dir = './smarty/configs';

$smarty->assign('name', 'theme');
$smarty->display('index.tpl');

?>

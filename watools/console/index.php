<?
require_once('./themes.php');

$smarty->assign('theme', $themeName);
$smarty->display('index.html');

?>

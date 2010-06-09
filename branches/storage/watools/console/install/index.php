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

// detect browser language
$defNames = array('en' => 'English', 'ru' => 'Русский');
$langs = array('en');
$lNames = array('en' => 'English');
$defLang = 'en';
$df = '';
if (isset($_SERVER['HTTP_ACCEPT_LANGUAGE'])) {
    $slangs=explode(',',$_SERVER['HTTP_ACCEPT_LANGUAGE']);
    //start going through each one
    foreach ($slangs as $value){
        $choice=substr($value,0,2);
        if(! in_array($choice, $langs)){
            $langs[] = $choice;
            if (isset($defNames[$choice])) {
                $lNames[$choice] = $defNames[$choice];
            }
            else {
                $lNames[$choice] = $choice;
            }
        }
        if ($df == '') {
            $df = $choice;
        }
    }
}
if ($df == "en" || $df == '') {
    $df = '';
    $defLang = 'en';
}
else {
    $defLang = $df;
    $df = '.' . $df;
}

$smarty->assign('langs', $langs);
$smarty->assign('lnNames', $lNames);
$smarty->assign('defLang', $defLang);
$smarty->display('index.html' . $df);
?>
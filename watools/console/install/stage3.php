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

$gDbDsn = $_POST['db_dsn'];
$gDbUsr = $_POST['db_usr'];
$gDbPwd = $_POST['db_pwd'];
$adminUser  = $_POST['user'];
$adminGroup = $_POST['group'];
$adminPswd  = $_POST['pass'];

CreateGroup($adminGroup, 'System Administrators');
CreateUser($adminUser, 'System Administrator', $adminPswd);
AddUserToGroup($adminUser, $adminGroup);

$path = realpath("..");
$npath = preg_replace('/\\\\/','\\\\\\\\',$path);
$code = "<?\n";
$code .= "\$gBaseDir = '$npath\\\\';\n";
$code .= "\$gDefaultTheme = 'sandbox';\n";
$code .= "\$gDbDsn = '$gDbDsn';\n";
$code .= "\$gDbUsr = '$gDbUsr';\n";
$code .= "\$gDbPwd = '$gDbPwd';\n";
$code .= "?>\n";

$code = highlight_string ($code, TRUE);

$smarty->assign('srvpath', $path);
$smarty->assign('code', $code);
$smarty->display('stage3.html' . $lang);
?>
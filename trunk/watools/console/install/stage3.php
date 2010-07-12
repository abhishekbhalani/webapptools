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

// search for themes
$db = GetDbConnection();
$table = GetTableName("themes");
$tid = DbExec($db, "SELECT rowid FROM $table");
if (is_null($tid[0])) {
	$tid = 0;
}
else {
	$tid = $tid[0];
}
$dirs = glob('../*', GLOB_ONLYDIR);
if ($dirs) {
	foreach ($dirs as $dir) {
		$files = glob($dir . '/theme.txt');
		if ($files) {
			$tname = substr($dir, 3);
			$handle = @fopen($files[0], "r");
			if ($handle) {
				$locstr = fgets($handle);
				$locstr = rtrim($locstr);
				$data = "";
				while (!feof($handle)) {
					$data .= fgets($handle);
				}
				fclose($handle);
				$locales = explode('|', $locstr);
				$q = DbExec($db, "SELECT * FROM $table WHERE name='tname'");
				if (is_null($q[0])) {
					// create theme
					$s = $db->prepare("INSERT INTO $table (id,name,desc) VALUES (?,?,?)");
					if ($s == false) {
						PrintDbError($db);
					}
					else {
						$s->bindParam(1, $tid);
						$s->bindParam(2, $tname);
						$s->bindParam(3, $data);
						$s->execute();
					}
					//foreach ($locales as $loc) {
					//	$r->sadd("Theme:$tid:Locales", $loc);    
					//}
					$tid++;
				}
			} // file handle opened
		} // any files found
	} // foraech dirs
} // any dirs found
// ... and set the defaults!
$table = GetTableName("_internals_");
DbExec($db, "REPLACE INTO $table (name,value) VALUES('DefaultTheme','sandbox')");
DbExec($db, "REPLACE INTO $table (name,value) VALUES('DefaultLang','en')");

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
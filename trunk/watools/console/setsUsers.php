<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('dashboard')) {
    $smarty->assign('UserName', $gUser[0]);
    $smarty->assign('theme', $themeName);
    $smarty->display('noaccess.html' . $themeLangExt);
    exit(0);
}

echo "<h1>Users &amp; Groups</h1>\n";
?>
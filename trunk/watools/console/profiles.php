<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('dashboard')) {
    PrintNoAccess();
    exit(0);
}

echo "<h1>Scanning profiles</h1>\n";
?>
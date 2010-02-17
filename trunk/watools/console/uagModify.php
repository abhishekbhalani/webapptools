<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');


// todo check ACL for access
if (!CheckACL('usermanagament')) {
    echo gettext('Access denied');
    exit(0);
}

$r = GetRedisConnection();
if (is_null($r)) {
    echo gettext('Database access error!');
    exit(0);
}

$action = $_POST['object'];
if ($action == 'user') {
}
else if ($action == 'group') {
    $msg = "OK";
    $gid = $_POST['gid'];
    $gname = $_POST['gname'];
    $gdesc = $_POST['gdesc'];
    if (!is_int($gid)) {
        $gid = -1;
    }
//$msg = "OK: $gid, $gname, $gdesc";
//echo $msg;
//exit(0);
    if ($gid == -1) {
        // add new group
        $res = CreateGroup($gname, $gdesc);
        if ($res == -2) {
            $msg .= gettext('Group with same name already exist: ') . $gname;
        }
        else if ($res < 1) {
            $msg .= gettext('Can\'t create group!');
        }
    }
    else {
        // modify existing group
        $r->lset("Group:$gid", $gname, 0);
        $r->lset("Group:$gid", $gdesc, 1);
    }
    echo $msg;
    exit(0);
}
else if ($action == 'userdel') {
}
else if ($action == 'groupdel') {
}
else {
    echo gettext('Invalid request!');
}
?>
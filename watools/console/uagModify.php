<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');


// todo check ACL for access
if (!CheckACL('usermanagament')) {
    echo gettext('Access denied');
    exit(0);
}

$r = GetDbConnection();
if (is_null($r)) {
    echo gettext('Database access error!');
    exit(0);
}

$action = $_POST['object'];
if ($action == 'user') {
    $msg = "OK";
    $uid = $_POST['uid'];
    $uname = $_POST['uname'];
    $udesc = $_POST['udesc'];
    $pwd = $_POST['pwd'];
    $grp = $_POST['grp'];
    $uid *= 2;
    $uid /= 2;
    
    if ($uid == -1) {
        if ($uname != "") {
            $res = CreateUser($uname, $udesc, $pwd);
            if ($res == -2) {
                $msg .= gettext('User with same name already exist: ') . $uname;
            }
            else if ($res < 1) {
                $msg .= gettext('Can\'t create user!');
            }
            else {
                if (!AddUserToGroupID($uname, $grp)) {
                    //$msg .= gettext('Can\'t add user to group! ') . $RedisError;
                }
            }
        }
        else {
            $msg = gettext('Invalid request!');
        }
    }
    else {
        // edit user
        $res = ModifyUser($uid, $uname, $udesc, $pwd);
/*        RemoveUserGroup($uid, $grp);
        if ($r->sismember("User:$uid:Groups", $grp) == 0) {
            $res = $r->sadd("User:$uid:Groups", $grp);
        }
        if ($r->sismember("Group:$grp:Members", $uid) == 0) {
            $res = $r->sadd("Group:$grp:Members", $uid);
        }*/
        
    }
    echo $msg;
    exit(0);
}
else if ($action == 'group') {
    $msg = "OK";
    $gid = $_POST['gid'];
    $gname = $_POST['gname'];
    $gdesc = $_POST['gdesc'];
    $gid *= 2;
    $gid /= 2;

    if ($gid <= 0) {
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
        $data = $r->lrange("Group:$gid", 0, 1);
        $r->delete("GroupName:" . $data[0]);
        $r->set("GroupName:" . $gname, $gid);
        $r->lset("Group:$gid", $gname, 0);
        $r->lset("Group:$gid", $gdesc, 1);
    }
    echo $msg;
    exit(0);
}
else if ($action == 'userdel') {
    $msg = "OK";
    $uid = $_POST['uid'];
    $uid *= 2;
    $uid /= 2;

    if ($uid > 0) {
        if ($uid == 1) {
            $msg = gettext("Can't delete system owner!");
        }
        else {
            DeleteUser($uid);
        }
    }
    else {
        $msg = gettext('Invalid request!');
    }
    echo $msg;
    exit(0);
}
else if ($action == 'groupdel') {
    $msg = "OK";
    $gid = $_POST['gid'];
    $gid *= 2;
    $gid /= 2;

    if ($gid > 0) {
        if ($gid == 1) {
            $msg = gettext("Can't delete system group!");
        }
        else {
            DeleteGroup($gid);
        }
    }
    else {
        $msg = gettext('Invalid request!');
    }
    echo $msg;
    exit(0);
}
else if ($action == 'userself') {
    $msg = gettext('Invalid request!');
    $desc = $_POST['desc'];
    $opwd = $_POST['opwd'];
    $pwd1 = $_POST['pwd1'];
    $pwd2 = $_POST['pwd2'];
    
    if (!is_null($gUser)) {
        $uid = $gUser['id'];
        if ($opwd == "") {
            $gUser[1] = $desc;
            $r->lset("User:$uid", $desc, 1);
            $msg = "OK";
        }
        else {
            if (CheckUserPassword($opwd)) {
                if ($pwd1 == $pwd2) {
                    $res = ModifyUser($uid, $gUser[0], $desc, $pwd1);
                    $msg = "OK";
                }
                else {
                    $msg = gettext('New password doesn\'t match the confirmation!');
                }
            }
            else {
                $msg = gettext('Invalid password!');
            }
        }
    }
    
    echo $msg;
    exit(0);
}
else {
    echo gettext('Invalid request!');
}
?>
<?
require_once('globals.php');
require_once('redisDB.php');

function CreateGroup($groupName, $groupDesc)
{
    $gid = -1;
    $r = GetRedisConnection();
    if (!is_null($r)) {
        if ($r->exists("GroupName:$groupName") == 0) {
            // create group
            $gid = $r->incr('Global:GroupID');
            SetValue($r, "GroupName:$groupName", $gid);
            $r->delete("Group:$gid");
            $msg = $r->rpush("Group:$gid", $groupName);
            if ($msg != "OK") {
                trigger_error(gettext("Can't save") . " Group:$gid => $msg.", E_USER_ERROR);
            }
            $msg = $r->rpush("Group:$gid", $groupDesc);
            if ($msg != "OK") {
                trigger_error(gettext("Can't save") . " Group:$gid => $msg.", E_USER_ERROR);
            }
        }
        else {
            $gid = -2;
        }
    }
    return $gid;
}

function DeleteGroup($gid)
{
    $result = NULL;
    
    $r = GetRedisConnection();
    if (!is_null($r)) {
        if ($r->exists("Group:$gid") == 1) {
            $nm = $r->lrange("Group:$gid", 0, 1);
            $mems = $r->smembers("Group:$gid:Members");
            foreach ($mems as $m) {
                RemoveUserGroup($m, $gid);
            }
            $r->delete("Group:$gid:Members");
            $r->delete("Group:$gid");
            $r->delete("GroupName:". $nm[0]);
            $result = true;
        }
    }
    return $result;
}
function CreateUser($userName, $userDesc, $userPwd)
{
    $uid = -1;
    
    $r = GetRedisConnection();
    if (!is_null($r)) {
        if ($r->exists("Login:$userName") == 0) {
            // create user
            $uid = $r->incr('Global:UserID');
            SetValue($r, "Login:$userName", $uid);
            $r->delete("User:$uid");
            $msg = $r->rpush("User:$uid", $userName);
            if ($msg != "OK") {
                trigger_error(gettext("Can't save") . " User:$uid => $msg.", E_USER_ERROR);
            }
            $msg = $r->rpush("User:$uid", $userDesc);
            if ($msg != "OK") {
                trigger_error(gettext("Can't save") . " User:$uid => $msg.", E_USER_ERROR);
            }
            $pwd = md5($userPwd);
            $msg = $r->rpush("User:$uid", $pwd);
            if ($msg != "OK") {
                trigger_error(gettext("Can't save") . " User:$uid => $msg.", E_USER_ERROR);
            }
        }
        else {
            $uid = -2;
        }
    }
    return $uid;
}

function DeleteUser($uid)
{
    $r = GetRedisConnection();
    if (!is_null($r)) {
        if ($r->exists("User:$uid") == 1) {
            $data = $r->lrange("User:$uid", 0, 1);
            $r->delete("Login:" . $data[0]);
            $mems = $r->smembers("User:$uid:Groups");
            foreach($mems as $m) {
                if ($r->exists("Group:$m:Members") == 1) {
                     $r->srem("Group:$m:Members", $uid);
                }
            }
            $r->delete("User:$uid");
            $r->delete("User:$uid:Groups");
        }
    }    
}

function ModifyUser($uid, $userName, $userDesc, $userPwd)
{
    $r = GetRedisConnection();
    if (!is_null($r)) {
        $data = $r->lrange("User:$uid", 0, 1);
        $r->delete("Login:" . $data[0]);
        $r->set("Login:" . $userName, $uid);
        $r->lset("User:$uid", $userName, 0);
        $r->lset("User:$uid", $userDesc, 1);
        if ($userPwd != "") {
            $pwd = md5($userPwd);
            $r->lset("User:$uid", $pwd, 2);
        }
    }
    return true;
}
function AddUserToGroup($userName, $groupName)
{
    global $RedisError;
    $result = false;

    $r = GetRedisConnection();
    if (!is_null($r)) {
        $uid = -1;
        $gid = -1;
        if ($r->exists("Login:$userName") == 1) {
            $uid = $r->get("Login:$userName");
        }
        if ($r->exists("GroupName:$groupName") == 1) {
            $gid = $r->get("GroupName:$groupName");
        }
        if ($uid > -1 && $gid > -1) {
            // add user to group
            if ($r->sismember("User:$uid:Groups", $gid) == 0) {
                $msg = $r->sadd("User:$uid:Groups", $gid);
                if ($msg == 0) {
                    $RedisError = gettext("Can't save") . " User:$uid:Groups => $msg.";
                    return $result;
                }
            }
            if ($r->sismember("Group:$gid:Members", $uid) == 0) {
                $msg = $r->sadd("Group:$gid:Members", $uid);
                if ($msg == 0) {
                    $RedisError = gettext("Can't save") . " Group:$gid:Members => $msg.";
                    return $result;
                }
            }
            $RedisError = "";
            $result = true;
        }
        else {
            $RedisError = gettext("Can't find User or Group!");
        }
    }
    
    return $refult;
}

function AddUserToGroupID($userName, $groupID)
{
    global $RedisError;
    $result = false;

    $r = GetRedisConnection();
    if (!is_null($r)) {
        $uid = -1;
        $gid = -1;
        if ($r->exists("Login:$userName") == 1) {
            $uid = $r->get("Login:$userName");
        }
        if ($r->exists("Group:$groupID") == 1) {
            $gid = $groupID;
        }
        if ($uid > -1 && $gid > -1) {
            // add user to group
            if ($r->sismember("User:$uid:Groups", $gid) == 0) {
                $msg = $r->sadd("User:$uid:Groups", $gid);
                if ($msg == 0) {
                    $RedisError = gettext("Can't save") . " User:$uid:Groups => $msg.";
                    return $result;
                }
            }
            if ($r->sismember("Group:$gid:Members", $uid) == 0) {
                $msg = $r->sadd("Group:$gid:Members", $uid);
                if ($msg == 0) {
                    $RedisError = gettext("Can't save") . " Group:$gid:Members => $msg.";
                    return $result;
                }
            }
            $RedisError = "";
            $result = true;
        }
        else {
            $RedisError = gettext("Can't find User or Group!");
        }
    }
    
    return $refult;
}

function GetGroupByName($groupName)
{
    $result = NULL;
    
    $r = GetRedisConnection();
    if (!is_null($r)) {
        if ($r->exists("GroupName:$groupName") == 1) {
            $gid = $r->get("GroupName:$groupName");
            $result = $r->lrange("Group:$gid", 0, 1);
            $result['id'] = $gid;
            $mems = $r->smembers("Group:$gid:Members");
            $result['members'] = $mems;
        }
    }
    return $result;
}

function RemoveUserGroup($uid, $gid)
{
    $r = GetRedisConnection();
    if (!is_null($r)) {
        if ($r->exists("User:$uid") == 1) {
            if ($r->exists("User:$uid:Groups") == 1) {
                 $r->sdel("User:$uid:Groups", $gid);
            }
        }
    }
}
function GetGroupByID($gid)
{
    $result = NULL;
    
    $r = GetRedisConnection();
    if (!is_null($r)) {
        if ($r->exists("Group:$gid") == 1) {
            $result = $r->lrange("Group:$gid", 0, 1);
            $result['id'] = $gid;
            $mems = $r->smembers("Group:$gid:Members");
            $result['members'] = $mems;
        }
    }
    return $result;
}

function GetUserByName($userName)
{
    $result = NULL;
    
    $r = GetRedisConnection();
    if (!is_null($r)) {
        if ($r->exists("Login:$userName") == 1) {
            $uid = $r->get("Login:$userName");
            $result = $r->lrange("User:$uid", 0, 2);
            $result['id'] = $uid;
            $mems = $r->smembers("User:$uid:Groups");
            $result['groups'] = $mems;
        }
    }
    return $result;
}

function GetUserByID($uid)
{
    $result = NULL;
    
    $r = GetRedisConnection();
    if (!is_null($r)) {
        if ($r->exists("User:$uid") == 1) {
            $result = $r->lrange("User:$uid", 0, 2);
            $result['id'] = $uid;
            $mems = $r->smembers("User:$uid:Groups");
            $result['groups'] = $mems;
        }
    }
    return $result;
}

function GetUserGroups($uid)
{
    $result = null;
    $usr = GetUserByID($uid);
    if (!is_null($usr)) {
        if ($usr['groups']) {
            $result = array();
            foreach ($usr['groups'] as $g) {
                $grp = GetGroupByID($g);
                if ($grp) {
                    $result[] = $grp[0];
                }
            }
        }
    }
    return $result;
}

function CheckACL($object)
{
    global $gSession, $gUser;
    $result = false;
    
    if (is_null($gUser)) {
        // not authenticated session
        return false;
    }
    
    return true;
}

?>
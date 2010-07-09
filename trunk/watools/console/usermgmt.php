<?
require_once('globals.php');
require_once('db.php');

function CreateGroup($groupName, $groupDesc)
{
    $gid = -1;
    $r = GetDbConnection();
    if (!is_null($r)) {
		// check existense
		$q = GetSingleRow($r, "SELECT * FROM gui_groups WHERE name='$groupName'");
		if (is_null($q)) {
			// create group
			$q = GetSingleRow($r, "SELECT max(rowid) FROM gui_groups");
			$id = $q[0];
			if (is_null($id)) { $id = 0; }
			$s = $r->prepare("INSERT INTO gui_groups (id, name, desc) VALUES (?, ?, ?)");
			if ($s == false) {
				echo $r->errorInfo();
			}
			else {
				$s->bindParam(1, $id);
				$s->bindParam(2, $groupName);
				$s->bindParam(3, $groupDesc);
				if ( ! $s->execute()) {
					$e = $r->errorInfo();
					echo $e[2]. "\n";
				}
				$q = GetSingleRow($r, "SELECT id FROM gui_groups WHERE name='$groupName'");
				if (!is_null($q)) {
					$gid = $q[0];
				}
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
    
    $r = GetDbConnection();
    if (!is_null($r)) {
		$r->exec("DELETE FROM gui_groups WHERE id=$gid");
		$r->exec("DELETE FROM gui_membership WHERE group_id=$gid");
    }
    return $result;
}

function CreateUser($userName, $userDesc, $userPwd)
{
    $uid = -1;
    
    $r = GetDbConnection();
    if (!is_null($r)) {
		// check existense
		$q = GetSingleRow($r, "SELECT * FROM gui_users WHERE login='$userName'");
		if (is_null($q)) {
		// create user
			$q = GetSingleRow($r, "SELECT max(rowid) FROM gui_users");
			$id = $q[0];
			if (is_null($id)) { $id = 0; }
			$s = $r->prepare("INSERT INTO gui_users (id, login, desc, password) VALUES (?, ?, ?, ?)");
			$pwd = md5($userPwd);
			if ($s == false) {
				$e = $r->errorInfo();
				echo $e[2];
			}
			else {
				$s->bindParam(1, $id);
				$s->bindParam(2, $userName);
				$s->bindParam(3, $userDesc);
				$s->bindParam(4, $pwd);
				if ( ! $s->execute()) {
					$e = $r->errorInfo();
					echo $e[2]. "\n";
				}
				$q = GetSingleRow($r, "SELECT * FROM gui_users WHERE login='$userName'");
				if (!is_null($q)) {
					$uid = $q[0];
				}
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
    $r = GetDbConnection();
    if (!is_null($r)) {
		$r->exec("DELETE FROM gui_users WHERE id=$uid");
		$r->exec("DELETE FROM gui_membership WHERE user_id=$uid");
    }    
}

function ModifyUser($uid, $userName, $userDesc, $userPwd)
{
    $r = GetDbConnection();
    if (!is_null($r)) {
		$s = $r->prepare("UPDATE gui_users SET login=?, desc=?, password=? WHERE id=$uid");
		$pwd = md5($userPwd);
		$s->bindParam(1, $userName);
		$s->bindParam(3, $userDesc);
		$s->bindParam(3, $pwd);
		$s->execute();
    }
    return true;
}

function AddUserToGroup($userName, $groupName)
{
    global $gDbError;
    $result = false;

    $r = GetDbConnection();
    if (!is_null($r)) {
        $uid = -1;
        $gid = -1;
		$q = GetSingleRow($r, "SELECT * FROM gui_users WHERE login='$userName'");
		if (!is_null($q)) {
			$uid = $q['id'];
		}
		$q = GetSingleRow($r, "SELECT * FROM gui_groups WHERE name='$groupName'");
		if (!is_null($q)) {
			$gid = $q['id'];
		}
        if ($uid > -1 && $gid > -1) {
            // add user to group
			$q = GetSingleRow($r, "SELECT * FROM gui_membership WHERE user_id=$uid AND group_id=$gid");
			if (is_null($q)) {
				$r->exec("INSERT INTO gui_membership (user_id, group_id) VALUES ($uid, $gid)");
			}
            $result = true;
        }
        else {
            $gDbError = gettext("Can't find User or Group!");
        }
    }
    return $refult;
}

function AddUserToGroupID($userName, $groupID)
{
    global $gDbError;
    $result = false;

    $r = GetDbConnection();
    if (!is_null($r)) {
        $uid = -1;
        $gid = -1;
		$q = GetSingleRow($r, "SELECT * FROM gui_users WHERE login='$userName'");
		if (!is_null($q)) {
			$uid = $q['id'];
		}
		$gid = $groupID;
		
        if ($uid > -1 && $gid > -1) {
            // add user to group
			$q = GetSingleRow($r, "SELECT * FROM gui_membership WHERE user_id=$uid AND group_id=$gid");
			if (is_null($q)) {
				$r->exec("INSERT INTO gui_membership (user_id, group_id) VALUES ($uid, $gid)");
			}
            $result = true;
        }
        else {
            $gDbError = gettext("Can't find User or Group!");
        }
    }
    return $refult;
}

function GetGroupByName($groupName)
{
    $result = NULL;
    
    $r = GetDbConnection();
    if (!is_null($r)) {
		$q = GetSingleRow($r, "SELECT * FROM gui_groups WHERE name='$groupName'");
        if (!is_null($r)) {
			$result = $r;
			// add query for members
			// $grp['members']
        }
    }
    return $result;
}

function RemoveUserGroup($uid, $gid)
{
    $r = GetDbConnection();
    if (!is_null($r)) {
		$r->exec("DELETE FROM gui_membership WHERE user_id=$uid AND group_id=$gid");
    }
}
function GetGroupByID($gid)
{
    $result = NULL;
    
    $r = GetDbConnection();
    if (!is_null($r)) {
		$q = GetSingleRow($r, "SELECT * FROM gui_groups WHERE id='$gid'");
        if (!is_null($r)) {
			$result = $r;
			// add query for members
			// $grp['members']
        }
    }
    return $result;
}

function GetUserByName($userName)
{
    $result = NULL;
    
    $r = GetDbConnection();
    if (!is_null($r)) {
		$q = GetSingleRow($r, "SELECT * FROM gui_users WHERE login='$userName'");
        if (!is_null($r)) {
			$result = $r;
			// add query for membership
			// $usr['groups']
        }
    }
    return $result;
}

function GetUserByID($uid)
{
    $result = NULL;
    
    $r = GetDbConnection();
    if (!is_null($r)) {
		$q = GetSingleRow($r, "SELECT * FROM gui_users WHERE id='$uid'");
        if (!is_null($r)) {
			$result = $r;
			// add query for membership
			// $usr['groups']
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
    $acl = GetACL($object);
    
    if (in_array('read', $acl)) {
        // not authenticated session
        $result = true;
    }
    
    return $result;
}

function GetACL($object)
{
    global $gSession, $gUser;
    $result = array();
    
    if (!is_null($gUser)) {
        // authenticated session
        $result[] = 'read';
        if ($gUser['id'] == 1) {
            $result[] = 'write';
            $result[] = 'delete';
            $result[] = 'execute';
            $result[] = 'system';
        }
        if (in_array(1, $gUser['groups']) && !in_array('system', $result)) {
            $result[] = 'system';
        }
    }
    
    return $result;
}

function CheckUserPassword($pwd)
{
    global $gUser;
    $code = md5($pwd);
    if(is_null($gUser)) {
        return false;
    }
    if ($code != $gUser[2]) {
        return false;
    }
    return true;
}

?>
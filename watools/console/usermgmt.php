<?
require_once('globals.php');
require_once('db.php');

function CreateGroup($groupName, $groupDesc)
{
    $gid = -1;
    $r = GetDbConnection();
    if (!is_null($r)) {
		$table = GetTableName("groups");
		// check existense
		$q = GetSingleRow($r, "SELECT * FROM $table WHERE name='$groupName'");
		if (is_null($q)) {
			// create group
			$q = GetSingleRow($r, "SELECT max(rowid) FROM $table");
			$id = $q[0];
			if (is_null($id)) { $id = 0; }
			$s = $r->prepare("INSERT INTO $table (id, name, desc) VALUES (?, ?, ?)");
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
				$q = GetSingleRow($r, "SELECT id FROM $table WHERE name='$groupName'");
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
		$table = GetTableName("groups");
		$r->exec("DELETE FROM $table WHERE id=$gid");
		$table = GetTableName("membership");
		$r->exec("DELETE FROM $table WHERE group_id=$gid");
    }
    return $result;
}

function CreateUser($userName, $userDesc, $userPwd)
{
    $uid = -1;
    
    $r = GetDbConnection();
    if (!is_null($r)) {
		// check existense
		$table = GetTableName("users");
		$q = GetSingleRow($r, "SELECT * FROM $table WHERE login='$userName'");
		if (is_null($q)) {
		// create user
			$q = GetSingleRow($r, "SELECT max(rowid) FROM $table");
			$id = $q[0];
			if (is_null($id)) { $id = 0; }
			$s = $r->prepare("INSERT INTO $table (id, login, desc, password) VALUES (?, ?, ?, ?)");
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
				$q = GetSingleRow($r, "SELECT * FROM $table WHERE login='$userName'");
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
		$table = GetTableName("users");
		$r->exec("DELETE FROM $table WHERE id=$uid");
		$table = GetTableName("membership");
		$r->exec("DELETE FROM $table WHERE user_id=$uid");
    }    
}

function ModifyUser($uid, $userName, $userDesc, $userPwd)
{
    $r = GetDbConnection();
    if (!is_null($r)) {
		$table = GetTableName("users");
		$s = $r->prepare("UPDATE $table SET login=?, desc=?, password=? WHERE id=$uid");
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
		$tuser = GetTableName("users");
		$tgroup = GetTableName("groups");
		$tmems = GetTableName("membership");
        $uid = -1;
        $gid = -1;
		$q = GetSingleRow($r, "SELECT * FROM $tuser WHERE login='$userName'");
		if (!is_null($q)) {
			$uid = $q['id'];
		}
		$q = GetSingleRow($r, "SELECT * FROM $tgroup WHERE name='$groupName'");
		if (!is_null($q)) {
			$gid = $q['id'];
		}
        if ($uid > -1 && $gid > -1) {
            // add user to group
			$q = GetSingleRow($r, "SELECT * FROM $tmems WHERE user_id=$uid AND group_id=$gid");
			if (is_null($q)) {
				$r->exec("INSERT INTO $tmems (user_id, group_id) VALUES ($uid, $gid)");
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
		$tuser = GetTableName("users");
		$tmems = GetTableName("membership");
        $uid = -1;
        $gid = -1;
		$q = GetSingleRow($r, "SELECT * FROM $tuser WHERE login='$userName'");
		if (!is_null($q)) {
			$uid = $q['id'];
		}
		$gid = $groupID;
		
        if ($uid > -1 && $gid > -1) {
            // add user to group
			$q = GetSingleRow($r, "SELECT * FROM $tmems WHERE user_id=$uid AND group_id=$gid");
			if (is_null($q)) {
				$r->exec("INSERT INTO $tmems (user_id, group_id) VALUES ($uid, $gid)");
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
		$tgroup = GetTableName("groups");
		$q = GetSingleRow($r, "SELECT * FROM $tgroup WHERE name='$groupName'");
        if (!is_null($q[0])) {
			$result = $q;
			// query the membership
			$tuser = GetTableName("membership");
			$gid = $result['id'];
			$s = $r->query("SELECT user_id FROM $tuser WHERE group_id=$gid")->fetchAll(PDO::FETCH_COLUMN);
			$result ['members'] = $s;
        }
    }
    return $result;
}

function RemoveUserGroup($uid, $gid)
{
    $r = GetDbConnection();
    if (!is_null($r)) {
		$tmems = GetTableName("membership");
		$r->exec("DELETE FROM $tmems WHERE user_id=$uid AND group_id=$gid");
    }
}
function GetGroupByID($gid)
{
    $result = NULL;
    
    $r = GetDbConnection();
    if (!is_null($r)) {
		$tgroup = GetTableName("groups");
		$q = GetSingleRow($r, "SELECT * FROM $tgroup WHERE id='$gid'");
        if (!is_null($q[0])) {
			$result = $q;
			// query the membership
			$tuser = GetTableName("membership");
			$s = $r->query("SELECT user_id FROM $tuser WHERE group_id=$gid")->fetchAll(PDO::FETCH_COLUMN);
			$result ['members'] = $s;
        }
    }
    return $result;
}

function GetUserByName($userName)
{
    $result = NULL;
    
	$r = GetDbConnection();
	if (!is_null($r)) {
		$tuser = GetTableName("users");
		$s = $r->prepare("SELECT * FROM $tuser WHERE login=?");
		if ($s != false) {
			$s->bindParam(1, $userName);
			$s->execute();
			if ($s) {
				$d = $s->fetchAll();
				$q = $d[0];
				if (!is_null($q)) {
					$result = $q;
					// query the membership
					$tuser = GetTableName("membership");
					$uid = $q['id'];
					$s = $r->query("SELECT group_id FROM $tuser WHERE user_id=$uid")->fetchAll(PDO::FETCH_COLUMN);
					$result ['groups'] = $s;
				}
			}
		}
	}
    return $result;
}

function GetUserByID($uid)
{
    $result = NULL;
    
    $r = GetDbConnection();
    if (!is_null($r)) {
		$tuser = GetTableName("users");
		$q = GetSingleRow($r, "SELECT * FROM $tuser WHERE id=$uid");
        if (!is_null($q[0])) {
			$result = $q;
			// query the membership
			$tuser = GetTableName("membership");
			$s = $r->query("SELECT group_id FROM $tuser WHERE user_id=$uid")->fetchAll(PDO::FETCH_COLUMN);
			$result ['groups'] = $s;
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
        if ($gUser['id'] == 0) {
            $result[] = 'write';
            $result[] = 'delete';
            $result[] = 'execute';
            $result[] = 'system';
        }
        if (in_array(0, $gUser['groups']) && !in_array('system', $result)) {
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
    if ($code != $gUser['password']) {
        return false;
    }
    return true;
}

?>
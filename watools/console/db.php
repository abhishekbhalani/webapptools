<?
require_once('globals.php');

$gDbError = "";
$gDbConnect = NULL;

$dbPrefix = "";
$dbStruct = array (
	array("_internals_", "(name TEXT UNIQUE, value VARIANT)"),
	array("sessions", "(id TEXT NOT NULL UNIQUE, timeout INT NOT NULL, client_id INT, connection_id INT NOT NULL, theme TEXT, lang TEXT)"),
	array("users", "(id INT NOT NULL UNIQUE, login TEXT UNIQUE, desc TEXT, password TEXT, theme INT, lang TEXT)"),
	array("groups", "(id INT NOT NULL UNIQUE, name TEXT UNIQUE, desc TEXT)"),
	array("membership", "(user_id INT NOT NULL, group_id INT NOT NULL)"),
	array("themes", "(id INT NOT NULL UNIQUE, name TEXT UNIQUE, desc TEXT)")
);

function GetDbConnection()
{
    global $gDbDsn, $gDbUsr, $gDbPwd, $gDbConnect, $gDbError;
    
    $gDbError = "";
    if (is_null($gDbConnect)) {
        //create connection
		try {
			$gDbConnect = new PDO($gDbDsn, $gDbUsr, $gDbPwd);
		} catch (PDOException $e) {
			$gDbError = 'Connection failed: ' . $e->getMessage() . "<br/>\n";
			header("HTTP/1.0 200 OK");
			echo $gDbError;
		}
    }
    if ($gDbError != "") {
        $gDbConnect = NULL;
    }
    return $gDbConnect;
}

function PrintDbError($dbh)
{
	$err = $err = $dbh->errorInfo();
	$gDbError .= $err[0] . ": " . $err[2] . "<br/>\n";
	echo $gDbError;
	exit(0);
}

function GetSingleRow($dbh, $query)
{
	$q = $dbh->query($query);
	$ret = NULL;
	if ($q) {
		foreach($q as $row) {
			$ret = $row;
		}
		$q->closeCursor();
	}
	return $ret;
}

function DbExec($dbh, $query)
{
	global $gDbError;
	
	if (!is_null($dbh)) {
		$res = $dbh->exec($query);
		if ($res === false) {
			$err = $dbh->errorInfo();
			$gDbError .= $err[0] . ": " . $err[2] . "<br/>\n";
		}
	}
	else {
		$res = false;
		$gDbError .= "DB connection doesn't intialized yet!<br/>\n";
	}
	return $res;
}

function GetTableName($short)
{
	global $dbPrefix, $dbStruct;
	
	foreach ($dbStruct as $table) {
		if ($short == $table[0]) {
			if ($dbPrefix != "") {
				return $dbPrefix . "." . $short;
			}
			else {
				return $short;
			}
		}
	}
}

function CreateTables($dbh, $drop)
{
	global $gDbError, $dbPrefix, $dbStruct;

	$gDbError = "";
	foreach ($dbStruct as $table) {
		$name = $table[0];
		if ($dbPrefix != "")
		{
			$name = $dbPrefix . "." . $table[0];
		}
		if ($drop && $table[0] != "_internals_") {
			// drop all except _internals_
			DbExec($dbh, "DROP TABLE IF EXISTS $name");
		}
		DbExec($dbh, "CREATE TABLE IF NOT EXISTS $name $table[1]");
	}
}
?>

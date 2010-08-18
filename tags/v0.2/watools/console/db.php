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
	array("modules", "(id TEXT NOT NULL, instance INT NOT NULL, class INT NOT NULL, version TEXT, ipaddr TEXT, name TEXT, stamp INT, timeout INT, onrun INT, status TEXT)"),
	array("modules_info", "(module_id TEXT NOT NULL  UNIQUE , osname TEXT, mem_size TEXT, cpu_usage TEXT, disk_size TEXT, max_tasks INT, stamp INT NOT NULL  DEFAULT 0, timeout INT NOT NULL  DEFAULT 0)"),
	array("module_cmds", "(module_id TEXT NOT NULL , timestamp INT NOT NULL , cmd BLOB)"),
	array("task", "(id INT, profile_id INT, name TEXT,  status INT, completion INT, start_time TEXT, finish_time TEXT, ping_time TEXT, requests INT, processed_urls BLOB)"),
	array("profile", "(profile_id INT, name TEXT, type INT, value VARIANT)"),
	array("profile_ui", "(plugin_id TEXT NOT NULL, plugin_name TEXT NOT NULL, locale TEXT NOT NULL, ui_settings BLOB NOT NULL, ui_icon BLOB NOT NULL)"),
	array("scan_data", "(id INT, task_id INT, parent_id INT, object_url TEXT, scan_depth INT, resp_code INT, data_size INT, dnld_time INT, content_type TEXT)"),
	array("vulners", "(id INT NOT NULL, task_id INT NOT NULL, object_id INT, plugin_id TEXT, severity INT DEFAULT 0, vulner_id INT, params TEXT)"),
	array("vulner_desc", "(plugin_id TEXT NOT NULL, id INT NOT NULL, severity INT NOT NULL, title TEXT, short TEXT, desc BLOB, locale TEXT)"),
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
	$err = $dbh->errorInfo();
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

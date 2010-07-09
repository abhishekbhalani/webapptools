<?
require_once('globals.php');

$gDbError = "";
$gDbConnect = NULL;

function GetDbConnection()
{
    global $gDbDsn, $gDbUsr, $gDbPwd, $gDbConnect, $gDbError;
    
    $gDbError = "";
    if (is_null($gDbConnect)) {
        //create connection
		try {
			$gDbConnect = new PDO($gDbDsn, $gDbUsr, $gDbPwd);
		} catch (PDOException $e) {
			$gDbError = 'Connection failed: ' . $e->getMessage();
			header("HTTP/1.0 200 OK");
			echo $gDbError;
		}
    }
    if ($gDbError != "") {
        $gDbConnect = NULL;
    }
    return $gDbConnect;
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

?>

<?
header("HTTP/1.0 200 OK");

require_once('./../globals.php');
require_once('./../db.php');

$result = "";

$gDbDsn = $_POST['db_dsn'];
$gDbUsr = $_POST['db_usr'];
$gDbPwd = $_POST['db_pwd'];
$drop = $_POST['drop'];

$dbh = GetDbConnection();

// create database
CreateTables($dbh, $drop);
$result .= $gDbError;

if ($result == "") {
    $result = "SUCCESS";
}
echo $result;
?>
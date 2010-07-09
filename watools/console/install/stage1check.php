<?
$result = "";

header("HTTP/1.0 200 OK");

$dsn = $_POST['dsn'];
$user = $_POST['uname'];
$pswd = $_POST['pswd'];
$drop = $_POST['drop'];
$result = "";

try {
	$dbh = new PDO($dsn, $user, $pswd);
} catch (PDOException $e) {
    $result = 'Connection failed: ' . $e->getMessage();
}

// drop previous tables
if ($drop) {
	$dbh->exec("DROP TABLE IF EXISTS gui_sessions");
	$dbh->exec("DROP TABLE IF EXISTS gui_users");
	$dbh->exec("DROP TABLE IF EXISTS gui_groups");
	$dbh->exec("DROP TABLE IF EXISTS gui_membership");
	$dbh->exec("DROP TABLE IF EXISTS gui_themes");
	$dbh->exec("DROP TABLE IF EXISTS gui_modules");
}
// create tables
$dbh->exec("CREATE TABLE IF NOT EXISTS gui_sessions (id INT NOT NULL UNIQUE, timeout INT NOT NULL, client_id INT, connection_id INT NOT NULL, theme INT, leng TEXT)");
$dbh->exec("CREATE TABLE IF NOT EXISTS gui_users (id INT NOT NULL UNIQUE, login TEXT UNIQUE, desc TEXT, password TEXT, theme INT, lang TEXT)");
$dbh->exec("CREATE TABLE IF NOT EXISTS gui_groups (id INT NOT NULL UNIQUE, name TEXT UNIQUE, desc TEXT)");
$dbh->exec("CREATE TABLE IF NOT EXISTS gui_membership (user_id INT NOT NULL, group_id INT NOT NULL)");
$dbh->exec("CREATE TABLE IF NOT EXISTS gui_themes (id INT NOT NULL UNIQUE, name TEXT UNIQUE, desc TEXT)");

if ($result == "") {
    $result = "SUCCESS";
}
echo $result;
?>
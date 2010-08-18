<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

$uname = $_POST['uname'];
$paswd = $_POST['paswd'];
$isAjax = $_POST['ajax'];
//echo "$uname => $paswd => $isAjax";
function AuthError()
{
    global $isAjax;
    
    if ($isAjax == 'true') {
        echo gettext("Incorrect username or password!");
    }
    else {
        header('HTTP/1.0 302 FOUND');
        header('Location: /');
    }
    exit(0);
}

if (!is_null($gSession) && !is_null($gUser)) {
    // already authenticated
    if ($isAjax == 'true') {
        echo "SUCCESS";
        exit(0);
    }
    else {
        header('HTTP/1.0 302 FOUND');
        header('Location: /main.php');
        exit(0);
    }
}
// try to find user
$r = GetDbConnection();
if (is_null($r)) {
    if ($isAjax == 'true') {
        echo gettext("Database connection error! " .$gDbError);
    }
    else {
        header('HTTP/1.0 302 FOUND');
        header('Location: /maintance.php');
    }
    exit(0);
}
$user = GetUserByName($uname);
if (is_null($user)) {
    AuthError();
}

$pwd = md5($paswd);
if ($user['password'] != $pwd) {
    AuthError();
}
// set authentication to the session
$gSession['client_id'] = $user['id'];
SaveSession();
$gUser = $user;

// send responce
if ($isAjax == 'true') {
    echo "SUCCESS";
}
else {
    header('HTTP/1.0 302 FOUND');
    header('Location: /main.php');
}

?>
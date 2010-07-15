<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('tasks')) {
    PrintNoAccess();
    exit(0);
}
$tid = $_POST['tsk_id'];

function xml_highlight($s)
{        
    $s = htmlspecialchars($s);
    $s = preg_replace("#&lt;([/]*?)(.*)([\s]*?)&gt;#sU",
        "<font color=\"#0000FF\">&lt;\\1\\2\\3&gt;</font>",$s);
    $s = preg_replace("#&lt;([\?])(.*)([\?])&gt;#sU",
        "<font color=\"#800000\">&lt;\\1\\2\\3&gt;</font>",$s);
    $s = preg_replace("#&lt;([^\s\?/=])(.*)([\[\s/]|&gt;)#iU",
        "&lt;<font color=\"#808000\">\\1\\2</font>\\3",$s);
    $s = preg_replace("#&lt;([/])([^\s]*?)([\s\]]*?)&gt;#iU",
        "&lt;\\1<font color=\"#808000\">\\2</font>\\3&gt;",$s);
    $s = preg_replace("#([^\s]*?)\=(&quot;|')(.*)(&quot;|')#isU",
        "<font color=\"#800080\">\\1</font>=<font color=\"#FF00FF\">\\2\\3\\4</font>",$s);
    $s = preg_replace("#&lt;(.*)(\[)(.*)(\])&gt;#isU",
        "&lt;\\1<font color=\"#800080\">\\2\\3\\4</font>&gt;",$s);
    return nl2br($s);
}

$db = GetDbConnection();
$table = GetTableName("task");
$tscan = GetTableName("scan_data");
$tvuln = GetTableName("vulners");
$tvdesc = GetTableName("vulner_desc");

$t = $db->prepare("SELECT * FROM $table WHERE id=?");
if ($t) {
	$t->bindParam(1, $tid);
	$t->execute();
	$tinfo = $t->fetchAll();
	if ($tinfo) {
		$tinfo = $tinfo[0];
		$tid = $tinfo['id'];
		$start_tm = $tinfo['start_time'];
		$finish_tm = $tinfo['finish_time'];
		$ping_tm = $tinfo['ping_time'];
		$stm = get_timestamp($start_tm);
		$ftm = get_timestamp($finish_tm);
		if ($stm > 0 && ($ftm > $stm)) {
			$dur = $ftm - $stm;
			$dur_tm = "";
			if ($dur > 86400) { // 86400 = 24 * 60 * 60 - one day
				$dur_tm = int($dur / 86400) . gettext("day(s)") . " ";
				$dur  = $dur % 86400;
			}
			date_default_timezone_set('GMT');
			$dur_tm .= date("H:i:s", $dur);
		}
		else {
			$dur_tm = gettext("Undefined");
		}
		// scan speed
		$speed = gettext("Undefined");
		if ($tinfo['requests'] && $tinfo['requests'] > 0) {
			$sp = $tinfo['requests'] / ($ftm - $stm);
			$sp = number_format($sp, 3);
			$speed = $tinfo['requests'] . " " . gettext("request(s);") . " " . $sp . " ". gettext("requests per second");
		}
		// get task target
		// SELECT object_url FROM $tscan WHERE task_id=$tid ORDER BY id ASC LIMIT 1
		$target = $tinfo['name'];
		if (!$target || $target == "") {
			$q = $db->query("SELECT object_url FROM $tscan WHERE task_id=$tid ORDER BY id ASC LIMIT 1");
			if ($q) {
				$url = $q->fetchAll(PDO::FETCH_COLUMN);
				if ($url) {
					$target = $url[0];
					$target = preg_replace("/[^:\\/]+:\\/\\/([^\\/]+)\\/.*/", "$1", $target);
				}
			}
		}
		// get scan_data
		$scList = $db->query("SELECT * FROM $tscan WHERE task_id=$tid ORDER BY object_url ASC")->fetchAll();
		// get vulners
		$vulns = $db->query("SELECT * FROM $tvuln WHERE task_id=$tid ORDER BY object_id ASC")->fetchAll();
		$vlList = array();
		foreach($vulns as $vuln) {
			$v = array();
			$v[] = $vuln['object_id'];
			$v[] = $vuln['plugin_id'] . "/" . $vuln['vulner_id'];
			// get vulner title
			$title = gettext("Undefined");
			$vdq = $db->prepare("SELECT title FROM $tvdesc WHERE plugin_id=? AND id=? AND locale=?");
			if ($vdq) {
				$vdq->bindParam(1, $vuln['plugin_id']);
				$vdq->bindParam(2, $vuln['vulner_id']);
				$vdq->bindParam(3, $gSession['lang']);
				$vdq->execute();
				$vd = $vdq->fetchAll(PDO::FETCH_COLUMN);
				if ($vd) {
					$title = $vd[0];
				}
			}
			$v[] = $title;
			$v[] = $vuln['severity'];
			$v[] = xml_highlight($vuln['params']);
			
			$vlList[] = $v;
		}
		
		$smarty->assign('target', $target);
		$smarty->assign('scanStart', $start_tm);
		$smarty->assign('scanStop', $finish_tm);
		$smarty->assign('scanPing', $ping_tm);
		$smarty->assign('scanLen', $dur_tm);
		$smarty->assign('scanSpeed', $speed);

		$smarty->assign('scCount', count($scList));
		$smarty->assign('scdata', $scList);
		$smarty->assign('vlCount', count($vlList));
		$smarty->assign('vldata', $vlList);
		$smarty->assign('startTime', $tinfo['start_time']);
		DisplayThemePage('taskinfo.html');
	}
	else {
		print gettext("Can't find selected task!");
	}
}
else {
	PrintDbError($db);
}
/// functions
function get_timestamp($src)
{
	$dat = strtotime($src);
	if (!$dat) {
		$dat = 0;
	}
	return $dat;
}
?>
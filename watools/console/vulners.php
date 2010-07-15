<?
require_once('./sessions.php');
require_once('./themes.php');
require_once('./usermgmt.php');

// todo check ACL for access
if (!CheckACL('vulners')) {
    PrintNoAccess();
    exit(0);
}
$db = GetDbConnection();
$lang = $gSession['lang'];
$table = GetTableName("vulner_desc");
$vuln_tree = array();
$qv = $db->query("SELECT plugin_id, id, title FROM $table WHERE locale='$lang'");
if ($qv) {
	$vulns = $qv->fetchAll();
	foreach($vulns as $v) {
		$pid = $v['plugin_id'];
		$vid = $v['id'];
		$vtt = $v['title'];
		$vuln_tree[$pid]['id'] = $pid;
		if (! $vuln_tree[$pid]['name'] || $vuln_tree[$pid]['name'] == "") {
			$vuln_tree[$pid]['name'] = $pid;
		}
		if ($vid == -1) {
			$vuln_tree[$pid]['name'] = $vtt;
		}
		if ($vid > -1) {
			$vuln_tree[$pid]['vulns'][$vid]['id'] = $vid;
			$vuln_tree[$pid]['vulns'][$vid]['title'] = $vtt;
		}
	}
}
$plg = array();
$plg['id'] = "plugin_1";
$plg['name'] = "Plugin 1";
$vuln = array();
$vuln['id'] = 0;
$vuln['title'] = "Sample 1.0";
$plg['vulns'][] = $vuln;
$vuln = array();
$vuln['id'] = 1;
$vuln['title'] = "Sample 1.1";
$plg['vulns'][] = $vuln;
$vuln = array();
$vuln['id'] = 2;
$vuln['title'] = "Sample 1.2";
$plg['vulns'][] = $vuln;
$vuln_tree[$plg['id']] = $plg;

$plg = array();
$plg['id'] = "plugin_2";
$plg['name'] = "Plugin 2";
$vuln = array();
$vuln['id'] = 0;
$vuln['title'] = "Vulner 2.0";
$plg['vulns'][] = $vuln;
$vuln = array();
$vuln['id'] = 1;
$vuln['title'] = "Vulner 2.1";
$plg['vulns'][] = $vuln;
$vuln = array();
$vuln['id'] = 2;
$vuln['title'] = "Vulner 2.2";
$plg['vulns'][] = $vuln;
$vuln_tree[$plg['id']] = $plg;

// make vulner tree
$output = "";
if (count($vuln_tree) > 0) {
	foreach($vuln_tree as $plg) {
		$output .= "<li class='closed'><span class='folder' onclick='SelectPlugin(\"" . $plg['id'] . "\")'>" . $plg['name'] . "</span>";
		if (count($plg['vulns']) > 0) {
			$output .= "<ul>";
			foreach($plg['vulns'] as $vulner) {
				$output .= "<li><span class='file' onclick='SelectVuln(\"" . $plg['id'] . "\", \"" .$vulner['id']. "\")'>" . $vulner['title'] . "</span></li>";
			}
			$output .= "</ul>";
		}
		$output .= "</li>";
	}
}

$smarty->assign('vulntree', $output);
DisplayThemePage('vulners.html');
?>
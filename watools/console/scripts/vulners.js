var selectedPlugin = "";
var selectedVulner = "";

function SelectVuln(plg, vuln) {
	selectedPlugin = plg;
	selectedVulner = vuln;
	$("#vuln_edit").load('vulnEdit.php',
		{"plg_id": plg, "vuln_id": vuln},
		function(resp, status) {
			if (status != "success") {
				$("#vuln_edit").html($("#errorEdit").html());
			}
		}
	);
}

function SelectPlugin(plg) {
	selectedPlugin = plg;
}

function EditPlugin() {
	if (selectedPlugin != "") {
		alert("Edit description for plugin " + selectedPlugin);
	}
	return false;
}

function AddVulner() {
	if (selectedPlugin != "") {
		alert("Add new vulner for plugin " + selectedPlugin);
	}
	return false;
}

function DelVulner() {
	if (selectedVulner != "") {
		alert("Delete vulner " + selectedPlugin + "/" + selectedVulner);
		selectedVulner = "";
	}
	return false;
}
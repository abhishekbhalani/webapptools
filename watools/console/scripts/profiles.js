var selectedProfile = -1;
var profileName = "";

function selectProfile(pID, pNM) {
	selectedProfile = pID;
	profileName = pNM;
	$("#prof_edit").load('profEdit.php',
		{"action": "load", "prof_id":  pID},
		function(resp, status) {
			if (status != "success") {
				$("#prof_edit").html($("#errorEdit").html());
			}
		}
	);
}

function addProfileOk() {
	var dialog_buttons = {};
	dialog_buttons[btnOk] = function() {
							$(this).dialog('close');
						};
	$("#msgText").load('profEdit.php',
		{"action": "add", "prof_name":  profileName},
		function(resp, status, xreq) {
			if (status != "success") {
				$("#msgText").html(xreq.status + " " + xreq.statusText);
				$("#msgBox").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
			else if (resp != "OK") {
				$("#msgText").html(resp);
				$("#msgBox").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
			else {
				$("#mainView").load("profiles.php");
			}
		});
}

function addProfile() {
	var dialog_buttons = {};
	dialog_buttons[btnOk] = function() {
                $(this).dialog('close');
                addProfileOk();
            };
	dialog_buttons[btnCancel] = function() {
                $(this).dialog('close');
            };
	$("#newName").dialog({
        autoOpen: true,
        width: 300,
        modal: true,
        buttons: dialog_buttons
    });
}

function delProfileOk(pID) {
	var dialog_buttons = {};
	dialog_buttons[btnOk] = function() {
							$(this).dialog('close');
						};
	$("#msgText").load('profEdit.php',
		{"action": "del", "prof_id":  pID},
		function(resp, status, xreq) {
			if (status != "success") {
				$("#msgText").html(xreq.status + " " + xreq.statusText);
				$("#msgBox").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
			else if (resp != "OK") {
				$("#msgText").html(resp);
				$("#msgBox").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
			else {
				$("#mainView").load("profiles.php");
			}
		});
}

function delProfile() {
	if (selectedProfile != -1) {
		var toDelete = selectedProfile;
		$("#msgText").html(delConfirm1 + profileName + delConfirm2);
		var dialog_buttons = {};
		dialog_buttons[btnYes] = function() {
								$(this).dialog('close');
								delProfileOk(toDelete);
							};
		dialog_buttons[btnNo] = function() {
								$(this).dialog('close');
							};
		$("#msgBox").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
		selectedProfile = -1;
	}
}

function cloneProfileOk(pID) {
	var dialog_buttons = {};
	dialog_buttons[btnOk] = function() {
							$(this).dialog('close');
						};
	$("#msgText").load('profEdit.php',
		{"action": "clone", "prof_id":  pID, "prof_name":  profileName},
		function(resp, status, xreq) {
			if (status != "success") {
				$("#msgText").html(xreq.status + " " + xreq.statusText);
				$("#msgBox").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
			else if (resp != "OK") {
				$("#msgText").html(resp);
				$("#msgBox").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
			else {
				$("#mainView").load("profiles.php");
			}
		});
}

function cloneProfile() {
	if (selectedProfile != -1) {
		var toClone = selectedProfile;
		profileName = clonePref + profileName;
		$("#nameInput").val(profileName);
		var dialog_buttons = {};
		dialog_buttons[btnOk] = function() {
					$(this).dialog('close');
					cloneProfileOk(toClone);
				};
		dialog_buttons[btnCancel] = function() {
					$(this).dialog('close');
				};
		$("#newName").dialog({
			autoOpen: true,
			width: 300,
			modal: true,
			buttons: dialog_buttons
		});
		selectedProfile = -1;
	}

}

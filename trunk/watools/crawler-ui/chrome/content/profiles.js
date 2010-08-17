function drawProfiles() {
	try {
		prfs = parent.getProfiles();
	}
	catch(e) {
		prfs = new Array();
		//prfs = new Array(new Array(0, "Profile 1"), new Array(1, "Profile 2"), new Array(2, "Profile 3"));
	}
	$("#prof-list").find("tr").remove();
	if (prfs.length == 0) {
		$('#prof-list > tbody:last').after('<tr class="ui-state-error"><td><span style="float: left; margin-right: 0.3em;" class="ui-icon ui-icon-alert"></span>No profiles found!</td></tr>');
	}
	else {
		row_class = new Array("tr_link tr_even","tr_link tr_odd");
		row_idx = 0;
		for (var i in prfs) {
			var prfName = prfs[i][1];
			prfName = prfName.replace(/\'/g, "\\'")
			row = '<tr class="'+row_class[row_idx]+'"><td onclick="selectProfile(\'' + prfs[i][0] + '\', \''  + prfName + '\')">'+prfs[i][1]+'</td></tr>';
			var x = $('#prof-list tr:last').length;
			if (x > 0) {
				$('#prof-list tr:last').after(row);
			}
			else {
				$('#prof-list > tbody:last').after(row);
			}
			row_idx++;
			if (row_idx == row_class.length) {
				row_idx = 0;
			}
			/*__alert(prfs[i][0] + " = " + prfs[i][1]);*/
		}
		row = '<tr><td>&nbsp;<td></tr>';
		var x = $('#prof-list tr:last').length;
		if (x > 0) {
			$('#prof-list tr:last').after(row);
		}
		else {
			$('#prof-list > tbody:last').after(row);
		}
	}
}

function setCntlValue(nm, val) {
	var tp = $('#settings_view [name='+nm+']').attr('type');
	var vl = unescape(val);
	if (tp == undefined) {
		tp = $('#settings_view [name='+nm+']').attr('nodeName');
	}
	if (tp != undefined) {
		if (tp == 'checkbox' || tp == 'CHECKBOX') {
			if (val != '' && val != '0') {
				$("#settings_view [name="+nm+"]").attr('checked', true);
			}
			else {
				$("#settings_view [name="+nm+"]").attr('checked', false);
			}
		}
		else if (tp == 'radio') {
			var $radios = $('#settings_view [name='+nm+']');
			$radios.filter('[value='+vl+']').attr('checked', true);
		}
		else {
			$("#settings_view [name="+nm+"]").val(vl);
		}
	}
	var r = tp + ": " + nm + "=" + vl;
	return r;
}

var toTransform = 0;

function transformCallback (t) {
	if (toTransform > 0) {
		toTransform = toTransform - 1;
	}
	if (toTransform == 0) {
		setTimeout(function () {
			vals = top.getProfileValues(selectedProfile);
			for (i in vals) {
				v = escape(vals[i][1]);
				n = vals[i][0];
				var x = setCntlValue(n, v);
			}
		}, 500);
	}
}

function selectProfile(idx, name) {
	selectedProfile = idx;
	profileName = name;
//alert ("Selected profile #" + idx + " - " + name);
	str = parent.getProfileVar(idx, 'profile_name');
	$("#profile_name").val(str);
	$("#prof_id").val(selectedProfile);
	plg_active = parent.getProfileVar(idx, 'plugin_list');
	if (plg_active != undefined) {
		plg_active = plg_active.split(';');
	}
	else {
		plg_active = new Array();
	}
	plg_list = parent.getPluginList();
	$("#plg_checks").children().remove();
	toTransform = plg_list.length;
	for (i in plg_list) {
		p_id = plg_list[i][0];
		p_nm = plg_list[i][1];
		pos = jQuery.inArray( p_id, plg_active );
		if (pos > -1) {
			plg_status = " checked";
		}
		else {
			plg_status = "";
		}
		icon = plg_list[i][2];
		if (icon == null || icon == "") {
			icon = "";
		}
		else {
			icon = writeXpm(icon);
			if (icon != "") {
				icon += "&nbsp;";
			}
		}
		str = '<input type="checkbox" id="plg_' + p_id + '" ' + plg_status + '><span onclick="toggleShow2(\'' + p_id + '\')" class="tr_link"><b>' + icon + p_nm + '</b></span><br/>';
		$("#plg_checks").append(str);
		str = '<div class="ui-widget-header" style="padding-left: 10px; display: none;" id="sets_'+p_id+'_title">' +icon+p_nm +'</div>';
		str += '<div class="ui-widget-content" style="width 100%; display: none;" id="sets_'+p_id+'_code"></div>';
		$("#settings_view").append(str);
		str = parent.getPluginUI(p_id);
		if (str != "") {
			new Transformation().setXml(str).setXslt('ui_transform.xslt').setCallback(transformCallback).transform('sets_'+p_id+'_code');
		}
		else {
			$('#sets_'+p_id+'_code').html("<div class='ui-state-highlight' width='100%' align='center'>Plugin doesn't provide settings.</div>");
			toTransform--;
		}
	}
	// set profile values to controls
	// xslt transformation is async process, so - wait for completion
	//setTimeout(function() {
	//	vals = top.getProfileValues(idx);
	//	for (i in vals) {
	//		v = escape(vals[i][1]);
	//		setControlValue(vals[i][0], v);
	//	}
	//}, 1000);
}

function addProfileOk() {
	try {
		ret = top.addProfile(profileName);
		if (ret != "OK") {
			__alert(ret);
		}
		else {
			drawProfiles();
		}
	}
	catch(e) {
		__alert("Can't add profile:\n"+e);
	}
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
		open: function() {
			$(this).parent().css("overflow", "hidden");
			$(this).parent().find(".ui-dialog-buttonpane").css("overflow", "hidden")
		},
        buttons: dialog_buttons
    });
}

function delProfileOk(pID) {
	var dialog_buttons = {};
	dialog_buttons[btnOk] = function() {
							$(this).dialog('close');
						};
	resp = parent.delProfileDB(pID);
	if (resp != "OK") {
		$("#msgText").html(resp);
		$("#msgBox").dialog({
			autoOpen: true,
			width: 400,
			modal: true,
			open: function() {
				$(this).parent().css("overflow", "hidden");
				$(this).parent().find(".ui-dialog-buttonpane").css("overflow", "hidden")
			},
			buttons: dialog_buttons
		});
	}
	drawProfiles();
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
			open: function() {
				$(this).parent().css("overflow", "hidden");
				$(this).parent().find(".ui-dialog-buttonpane").css("overflow", "hidden")
			},
					buttons: dialog_buttons
				});
		selectedProfile = -1;
	}
}

function cloneProfileOk(pID) {
	try {
		ret = top.cloneProfile(pID, profileName);
		if (ret != "OK") {
			__alert(ret);
		}
		else {
			drawProfiles();
		}
	}
	catch(e) {
		__alert("Can't add profile:\n"+e);
	}
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
			open: function() {
				$(this).parent().css("overflow", "hidden");
				$(this).parent().find(".ui-dialog-buttonpane").css("overflow", "hidden")
			},
			buttons: dialog_buttons
		});
		selectedProfile = -1;
	}

}

var selectedProfile = -1;
var profileName = "";
var btnOk = "Ok";
var btnCancel = "Cancel";
var btnYes = "Yes";
var btnNo = "No";
var btnDrop = "Drop";
var btnApply = "Apply";
var btnRun = "Run";
var delConfirm1 = "Are you shure to delete profile ";
var delConfirm2 = "?<br><br><div class='ui-state-error' width='100%' align='center'>This operation can't be undone!</div>";
var clonePref = "Copy of ";

function SelectDB() {
	$("[name^='dbd_']").each(function(idx, domel){/*__alert(domel.attributes['name'].value);*/$(domel).hide()});
	idx=$("#db_drv").val();
	$("[name=dbd_"+idx+"]").show();
}

function SaveFile(flInp, realInp) {
	var t = $("#"+realInp);
	var f = $("#"+flInp);
	t.val(f.val());
}

function OpenDBFile(flInp, realInp) {
	SaveFile(flInp, realInp);
	var f = $("#"+flInp);
	parent.openDatabase(f.val(), $("#sqlite_chache").val());
	$("#sqlite_version").text(parent.GetDBVersion());
	drawProfiles();
	drawTasks();
}

function SavePrefs() {
	parent.setPref("crawler_executable", $("#crawler_file").val());
	parent.setPref("crawler_configuration", $("#crawler_cfg").val());
	parent.setPref("db_driver", $("#db_drv").val());
	parent.setPref("sqlite_database", $("#sqlite_file").val());
	parent.setPref("sqlite_chache", $("#sqlite_chache").val());
}

function LoadPrefs() {
	path = top.GetWD();
	os = top.GetOS();
	
	if (os == "WINNT") {
		exec = path+"\\crawler.exe";
		conf = path+"\\crawler.conf";
		dbfile = path+"\\database.sqlite";
	} else {
		exec = path+"/crawler";
		conf = "/etc/watools/crawler.conf";
		dbfile = path+"/database.sqlite";
	}

	$("#crawler_file").val(parent.getPref("crawler_executable", exec));
	$("#crawler_cfg").val(parent.getPref("crawler_configuration", conf));
	$("#db_drv").val(parent.getPref("db_driver", "sqlite"));
	$("#sqlite_file").val(parent.getPref("sqlite_database", dbfile));
	$("#sqlite_chache").val(parent.getPref("sqlite_chache", "2048"));

	/* reload data*/
	parent.openDatabase($("#sqlite_file").val(), $("#sqlite_chache").val());
	$("#sqlite_version").text(parent.GetDBVersion());
	SelectDB();
	drawProfiles();
	drawTasks();
}

function boostToDate(str) {
	var month_names = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
	var __ret= new Date(0,0,0,0,0,0);
//__alert (str);
	var drx = /(\d\d\d\d)-(\w\w\w)-(\d\d) (\d\d):(\d\d):(\d\d)/
	var sst = str.match(drx);
//__alert (sst);
	if (sst != null) {
		sst[2] = jQuery.inArray(sst[2], month_names);
		__ret = new Date(sst[1], sst[2], sst[3], sst[4], sst[5], sst[6]);
	}
//__alert(__ret);
	return __ret;
}

function makeDuration(tmDur) {
	s = tmDur % 60;
	tmDur -= s;
	m = (tmDur / 60) % 60;
	tmDur -= m * 60;
	h = (tmDur / 3600) % 24;
	tmDur -= h * 24;
	d = tmDur / 86400;
	m = m.toFixed(0);
	h = h.toFixed(0);
	d = d.toFixed(0);
	ss = "";
	if (d > 0) {
		ss = d + "d. ";
	}
	if (h < 10) {
		ss += "0";
	}
	ss += h + ":";
	if (m < 10) {
		ss += "0";
	}
	ss += m + ":";
	if (s < 10) {
		ss += "0";
	}
	ss += s;
	
	return ss;
}

function __alert(txt, title) {
	if (title) {
		$("#style_alert").attr("title", title);
	}
	txt = txt.replace(/\n/g, "<br>");
	$("#style_alert_text").html(txt);
	$('#style_alert').dialog({
		autoOpen: true,
		width: 400,
		modal: true,
		open: function() {
				$(this).parent().css("overflow", "hidden");
				$(this).parent().find(".ui-dialog-buttonpane").css("overflow", "hidden");
		},
		buttons: {
			"Ok": function() { $(this).dialog('close'); }
		}
	});
}

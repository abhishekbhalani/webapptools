var selectedTask = -1;

function goBack() {
	$("#task_list").show();
	$("#task_tree").hide();
	$("#task_info").hide();
	resizeTabPanelLayout();
}

function toggleView(view) {
	v = $('#'+view).attr('visible');
	if (v == 0) {
		$('#'+view).attr('visible', '1');
		$('#'+view).show();
		$('#'+view+"_btn").removeClass('ui-icon-plus');
		$('#'+view+"_btn").addClass('ui-icon-minus');
	}
	else {
		$('#'+view).attr('visible', '0');
		$('#'+view).hide();
		$('#'+view+"_btn").removeClass('ui-icon-minus');
		$('#'+view+"_btn").addClass('ui-icon-plus');
	}
	return false;
}


function drawTasks() {
	var tskList;
	try {
		tskList = top.getTaskList();
	}
	catch(e) {
		//tskList = new Array();
		tskList = new Array(new Array(0, 1, 2, 3, 4, 5, 6, 7 , 8, 9), new Array());
	}
	$("#tsk_list").find("*").remove();
	if (tskList.length == 0) {
		$("#task_none").show();
		$("#task_table_wrapper").hide();
	}
	else {
		$("#task_table_wrapper").show();
		$("#task_none").hide();
		var tbl = $("#task_table");
		tbl.find("tr:gt(0)").remove();
		row_class = new Array("tr_link tr_even","tr_link tr_odd");
		row_idx = 0;
		for (var i in tskList) {
			// fix task status
			// 0 - IDLE
			// 1 - RUNNING
			// 2 - PAUSED
			// -----------
			// 3 - FINISHED
			// 4 - STOPPED
			// 5 - HANGS
			ts = tskList[i][2];
			st = boostToDate(tskList[i][4]);
			ft = boostToDate(tskList[i][5]);
			pt = boostToDate(tskList[i][6]);
			if (ts != 1 && ft > st && tskList[i][3] == 100) {
				ts = 3; // completed
			}
			if (ts < 3 && ft > st && pt < ft && tskList[i][3] < 100) {
				// stopped
				ts = 4;
			}
			if (ts < 3 && ft.valueOf() == NaN && pt.valueOf() == NaN ) {
				// hangs
				ts = 5;
			}
			if (ts < 0 || ts > 5) {
				ts = 6;
			}
			tacts = "<span class='ui-icon ui-icon-circle-close' title='Delete Task' onclick='return deleteTask(\""+tskList[i][0]+"\");'></span>";
			tstat = "";
			if (ts == 0) {
				tstat = "<span class='ui-icon ui-icon-power'></span> IDLE";
			}
			else if (ts == 1) {
				tstat = "<span class='ui-icon ui-icon-play'></span> RUNNING";
				tacts = "<span class='ui-icon ui-icon-pause' title='Pause Task' onclick='return pauseTask(\""+tskList[i][0]+"\");'></span>";
				tacts += "<span class='ui-icon ui-icon-power' title='Stop Task' onclick='return stopTask(\""+tskList[i][0]+"\");'></span>";
			}
			else if (ts == 2) {
				tstat = "<span class='ui-icon ui-icon-pause'></span> PAUSED";
				tacts = "<span class='ui-icon ui-icon-seek-next' title='Resume Task' onclick='return resumeTask(\""+tskList[i][0]+"\");'></span>";
				tacts += "<span class='ui-icon ui-icon-power' title='Stop Task' onclick='return stopTask(\""+tskList[i][0]+"\");'></span>";
			}
			else if (ts == 3) {
				tstat = "<span class='ui-icon ui-icon-circle-check'></span> FINISHED";
			}
			else if (ts == 4) {
				tstat = "<span class='ui-icon ui-icon-stop'></span> STOPPED";
			}
			else if (ts == 5) {
				tstat = "<span class='ui-icon ui-icon-alert'></span> HANGS";
				tacts += "<span class='ui-icon ui-icon-power' title='Stop Task' onclick='return stopTask(\'"+tskList[i][0]+"\');'></span>";
			}
			else {
				tstat = "<span class='ui-icon ui-icon-cancel'></span> undefined";
			}

			// draw task row
			str  = '<tr class="'+row_class[row_idx]+'">'; // " onclick="showInfo(\''+tskList[i][0]+'\')"
			str += '<td onclick="showInfo(\''+tskList[i][0]+'\')">'+tskList[i][1]+'</td>';
			str += '<td onclick="showInfo(\''+tskList[i][0]+'\')">'+tskList[i][8]+'&nbsp;</td>';
			str += '<td onclick="showInfo(\''+tskList[i][0]+'\')" style="white-space: nowrap;">'+tstat+'&nbsp;</td>';
			str += '<td onclick="showInfo(\''+tskList[i][0]+'\')">'+tskList[i][3]+'%</td>';
			str += '<td onclick="showInfo(\''+tskList[i][0]+'\')">'+tskList[i][4]+'&nbsp;</td>';
			str += '<td onclick="showInfo(\''+tskList[i][0]+'\')">'+tskList[i][5]+'&nbsp;</td>';
			str += '<td onclick="showInfo(\''+tskList[i][0]+'\')">'+tskList[i][6]+'&nbsp;</td>';
			str += '<td>'+tacts+'&nbsp;</td>';
			str += '</tr>';
			$('#task_table tr:last').after(str);
			row_idx++;
			if (row_idx == row_class.length) {
				row_idx = 0;
			}
		}
	}
}

function showInfo(tsk_id) {
	selectedTask = tsk_id;

	$("#task_list").hide();
	$("#task_tree").hide();
	$("#task_info").show();
	resizeTabPanelLayout();
	var tsks;
	var objs;
	var vulns;
	try {
		tsks = top.getTask(tsk_id);
		objs = top.getTaskObjects(tsk_id);
		vulns = top.getTaskVulners(tsk_id);
	}
	catch(e) {
		//tsks = new Array(0, 1, 2, 3, 4, 5, 6, 7 , 8, 9);
		tsks = new Array();
		objs = new Array();
		vulns = new Array();
		__alert("Task loading failed: " + e);
	}
	$("#task_info_header").text('Task - '+ tsks[8] +' (started at '+tsks[4]+')');
	$("#scanStart").text(tsks[4]);
	$("#scanStop").text(tsks[5]);
	$("#scanPing").text(tsks[6]);
	var sd = boostToDate(tsks[4]);
	var fd = boostToDate(tsks[5]);
	var pd = boostToDate(tsks[6]);
	if (pd > fd) {
		fd = pd;
	}
	if (fd > sd) {
		sd = fd - sd;
		sd = sd / 1000;
		sd = sd.toFixed(0);
		slen = makeDuration(sd);
		if (tsks[9] > 0) {
			sp = tsks[9] / sd;
			sp = sp.toFixed(3);
			sp += " requests per second";
		}
		else {
			sp = "undefined";
		}
	}
	else {
		slen = "undefined";
		sp = "undefined";
	}
	$("#rqCount").text(tsks[9]);
	$("#scanLen").text(slen);
	$("[name=scCount]").text(objs.length);
	$("[name=vlCount]").text(vulns.length);
	$("#scanSpeed").text(sp);
	if (objs.length > 0) {
		$("#objects_blank").hide();
		$("#objects").show();
		$("#objects").find("tr:gt(0)").remove();
	}
	else {
		$("#objects_blank").show();
		$("#objects").hide();
	}
	if (vulns.length > 0) {
		$("#vulners_blank").hide();
		$("#vulners").show();
		$("#vulners").find("tr:gt(0)").remove();
	}
	else {
		$("#vulners_blank").show();
		$("#vulners").hide();
	}
	// draw objects
	for (var i in objs) {
		row = "<tr>";
		row += "<td>" +objs[i][2]+ "</td>";
		row += "<td>" +objs[i][4]+ "</td>";
		row += "<td>" +objs[i][5]+ "</td>";
		row += "<td>" +objs[i][7]+ "</td>";
		row += "</tr>";
		$('#objects tr:last').after(row);
	}
	// draw vulners
	for (var i in vulns) {
		row = "<tr>";
		row += "<td>" +vulns[i][1]+ "</td>";
		row += "<td>" +vulns[i][2]+ "/"+vulns[i][4]+"</td>";
		row += "<td>" +vulns[i][6]+ "</td>";
		row += "<td>" +vulns[i][3]+ "</td>";
		str = vulns[i][5];
		str = str.replace(/\</g, "&lt;");
		str = str.replace(/\>/g, "&gt;");
		row += "<td class='vuln_param'><code class='prettyprint'>" +str+ "</code></td>";
		row += "</tr>";
		$('#vulners tr:last').after(row);
	}
	prettyPrint();

	return false;
}


function stopTaskOk(tsk_id)
{
	__alert("... Just change task's status in the database...");
dump("stopTaskOk for " + tsk_id + "\n");
	top.setTskStatus(tsk_id, 4); //set to stopped
	drawTasks();
	return false;
}

function stopTask(tsk_id)
{
	var dialog_buttons = {};
	dialog_buttons[btnNo] = function() {
							$(this).dialog('close');
						};
	dialog_buttons[btnYes] = function() {
							$(this).dialog('close');
							stopTaskOk(tsk_id);
						};
    $('#confirm_stop').dialog({
		autoOpen: true,
		width: 400,
		modal: true,
		open: function() {
				$(this).parent().css("overflow", "hidden");
				$(this).parent().find(".ui-dialog-buttonpane").css("overflow", "hidden")
		},
		buttons: dialog_buttons
	});
	return false;
}

function deleteTaskOk(tsk_id)
{
	try {
		top.deleteTask(tsk_id);
	}
	catch (e) {
		__alert("Task deletion failed:\n" + e);
	}
	drawTasks();
	return false;
}

function deleteTask(tsk_id)
{
	var dialog_buttons = {};
	dialog_buttons[btnNo] = function() {
							$(this).dialog('close');
						};
	dialog_buttons[btnYes] = function() {
							$(this).dialog('close');
							deleteTaskOk(tsk_id);
						};
    $('#confirm_delete').dialog({
		autoOpen: true,
		width: 400,
		modal: true,
		open: function() {
				$(this).parent().css("overflow", "hidden");
				$(this).parent().find(".ui-dialog-buttonpane").css("overflow", "hidden")
		},
		buttons: dialog_buttons
	});
	return false;
}

function pauseTask(tsk_id)
{
	__alert("Not implemented yet!");
	return false;
}

function resumeTask(tsk_id)
{
	return false;
}

function setFilterBtn()
{
	if (use_filter == "0") {
		$('#task_filter').removeClass("ui-state-highlight");
		$('#task_filter').removeClass("ui-state-focus");
	}
	else {
		$('#task_filter').addClass("ui-state-highlight");
	}
}

function applyFilter()
{
	var str = $("#filter_form").serializeArray();
	str = JSON.stringify(str);
	str = btoa(str);
	top.setPref("filter", str);
	top.setPref("use_filter", use_filter);
	RefreshList();
	return false;
}

function getFilter()
{
	var fltSetting = top.getPref("filter", "");
	use_filter = top.getPref("use_filter", "0");
	if (fltSetting != "") {
		clearText = atob(fltSetting);
		if (clearText != "") {
			formData = JSON.parse(clearText);
			if (formData) {
				jQuery.each(formData, function(i, field){
					var tp = $("[name="+field.name+"]").attr('type');
					if (tp == "checkbox") {
						$("[name="+field.name+"]").attr('checked', true);
					}
					else {
						$("[name="+field.name+"]").val(field.value);
					}
				  });
			}
		}
	}
	return false;
}

function taskFilter()
{
	var dialog_buttons = {};
	dialog_buttons[btnDrop] = function() {
						$(this).dialog('close');
						use_filter = "0";
						setFilterBtn();
						applyFilter();
					};
	dialog_buttons[btnApply] = function() {
						$(this).dialog('close');
						use_filter = "1";
						setFilterBtn();
						applyFilter();
					};
	dialog_buttons[btnCancel] = function() {
						$(this).dialog('close');
					};
	$("#task_filter_edit").dialog({
		autoOpen: true,
		width: 500,
		modal: true,
		open: function() {
				$(this).parent().css("overflow", "hidden");
				$(this).parent().find(".ui-dialog-buttonpane").css("overflow", "hidden")
		},
		buttons: dialog_buttons
	}); 
	return false;
}

function addTaskOK()
{
	var exe = $("#crawler_file").val();
	if (exe != "") {
		// get the directory to accuire config
		// todo - move to Settings panel
		cfg = exe;
		cfg = cfg.replace(/(.*(\/|\\))crawler.*/i, "$1");
		cfg += "crawler.conf";
		//exe = '"' + exe + '"'
		var args = new Array();
		args.push("--config");
		args.push('"' + cfg + '"');
		
		args.push("--target");
		s = $("#task_target").val();
		s = '"' + s + '"';
		args.push(s);
		
		args.push("--name");
		s = $("#task_name").val();
		s = '"' + s + '"';
		args.push(s);
		
		args.push("--profile");
		s = $("#task_profile").val();
		s = s; //must be INT
		args.push(s);

		args.push("--output");
		args.push("2");
		
		args.push("--result");
		s = $("#task_target").val();
		s = s.replace(/(\w+:\/\/)?([^\/]+).*/, "$2");
		args.push('"' + s + '.txt"');
		
		dbg = exe + ' ';
		dbg += args.join(" ");
//__alert(dbg);
		try {
			top.runCrawler(exe, args);
		}
		catch (e) {
			__alert("Can't start crawler task:\n" + dbg + "\n" + e);
		}
	}
	drawTasks();
}

function addTask()
{
	prfs = top.getProfiles();
	$("#task_profile").find('option').remove("*");
	for (var i in prfs) {
		elem = '<option value="'+prfs[i][0]+'">'+prfs[i][1]+'</option>';
		$("#task_profile").append(elem);
	}
	var dialog_buttons = {};
	dialog_buttons[btnRun] = function() {
						$(this).dialog('close');
						addTaskOK();
					};
	dialog_buttons[btnCancel] = function() {
						$(this).dialog('close');
					};
	$("#task_runner").dialog({
		autoOpen: true,
		width: 400,
		modal: true,
		open: function() {
				$(this).parent().css("overflow", "hidden");
				$(this).parent().find(".ui-dialog-buttonpane").css("overflow", "hidden")
		},
		buttons: dialog_buttons
	}); 
	return false;
}

function textExport() {
	var txt = "";
	var tsks;
	var objs;
	var vulns;
	try {
		tsks = top.getTask(selectedTask);
		objs = top.getTaskObjects(selectedTask);
		vulns = top.getTaskVulners(selectedTask);
	}
	catch(e) {
		__alert("Task loading failed: " + e);
		return;
	}
	txt = 'Task - '+ tsks[8] +' (started at '+tsks[4]+')\n\n';
	txt += "Scan started:\t" + tsks[4] + "\n";
	txt += "Scan finished:\t" + tsks[5] + "\n";
	txt += "Last activity:\t" + tsks[6] + "\n";
	var sd = boostToDate(tsks[4]);
	var fd = boostToDate(tsks[5]);
	var pd = boostToDate(tsks[6]);
	if (fd > sd) {
		sd = fd - sd;
		sd = sd / 1000;
		sd = sd.toFixed(0);
		slen = makeDuration(sd);
		if (tsks[9] > 0) {
			sp = tsks[9] / sd;
			sp = sp.toFixed(3);
			sp += " requests per second";
		}
		else {
			sp = "undefined";
		}
	}
	else {
		slen = "undefined";
		sp = "undefined";
	}
	txt += "Scan duration:\t" + slen + "\n";
	txt += "Objects found:\t" + objs.length + "\n";
	txt += "Vulnerabilities found:\t" + vulns.length + "\n";
	txt += "Total requests:\t" + tsks[9] + "\n";
	txt += "Scan speed:\t" + sp + "\n";
	txt += "\n";
	// draw objects
	txt += "URL\tHTTP code\tDownload time\tSize\n";
	for (var i in objs) {
		txt += objs[i][2]+ "\t";
		txt += objs[i][4]+ "\t";
		txt += objs[i][6]+ "\t";
		txt += objs[i][5]+ "\n";
	}
	txt += "\n";
	// draw vulners
	txt += "Object ID\tTitle\tSeverity\tParameters\n";
	for (var i in vulns) {
		txt += vulns[i][1] + "\t";
		txt += vulns[i][6] + "\t";
		txt += vulns[i][3] + "\t";
		txt += vulns[i][5] + "\n";
	}
	//__alert("Export to text:\n" + txt);
	top.SaveTxtFile(txt);
}

function treeView()
{
	$("#task_list").hide();
	$("#task_tree").show();
	$("#task_info").hide();
	resizeTabPanelLayout();

	$("#tsk_tree_list").children().remove("*");
	var tsks;
	var objs;
	var vulns;
	try {
		tsks = top.getTask(selectedTask);
		objs = top.getTaskObjects(selectedTask);
		vulns = top.getTaskVulners(selectedTask);
	}
	catch(e) {
		//tsks = new Array(0, 1, 2, 3, 4, 5, 6, 7 , 8, 9);
		tsks = new Array();
		objs = new Array();
		vulns = new Array();
		__alert("Task loading failed: " + e);
	}
	$("#task_info_header2").text('Task - '+ tsks[8] +' (started at '+tsks[4]+')');
	objs.sort(function(a, b) {
		if (a[3] == b[3]) return 0;
		if (a[3] > b[3]) return 1;
		return -1;
	});
	site = objs[0][2];
	site = site.replace(/[^:\/]+:\/\/([^\/]+)\/.*/, "$1");
	$("#tsk_tree_list").append('<li id="node_0" class="tr_link" onclick="showObj(\'0\')">'+ site + '</li>');
	for (var i in objs) {
		id = Number(objs[i][0]);
		if (id == NaN || id < 1) {
			id = 0;
		}
		id = "node_" + id;
		prnt = Number(objs[i][1]);
		if (prnt == NaN || prnt < 1) {
			prnt = 0;
		}
		pid = "#node_" + prnt;
		url = objs[i][2];
		url = url.replace(/[^:\/]+:\/\/[^\/]+(\/.*)/, "$1");
		elem = '<li id="'+id+'" class="tr_link" onclick="showObj(\''+objs[i][0]+'\')"><span class="ui-icon ui-icon-document"></span>' + url + '</li>';
		ul = $(pid+" ul:first").length;
		//__alert(ul+" - "+pid + ": " + $(pid).html());
		if (ul == 0) {
			//__alert(pid + " adding list");
			$(pid).append('<ul></ul>');
		}
		$(pid+" > ul:first").append(elem);
	}
	for (var i in vulns) {
		id = Number(vulns[i][0]);
		if (id == NaN || id < 1) {
			id = 0;
		}
		id = "node_" + id;
		prnt = Number(vulns[i][1]);
		if (prnt == NaN || prnt < 1) {
			prnt = 0;
		}
		pid = "#node_" + prnt;
		elem = '<li id="'+id+'" class="tr_link" onclick="showVuln(\''+vulns[i][0]+'\')"><span class="ui-icon ui-icon-gear"></span>' + vulns[i][6] + '</li>';
		ul = $(pid+" ul:first").length;
		//__alert(ul+" - "+pid + ": " + $(pid).html());
		if (ul == 0) {
			//__alert(pid + " adding list");
			$(pid).append('<ul></ul>');
		}
		$(pid+" > ul:first").append(elem);
	}
	$("#tsk_tree_list").treeview();
}

var inTree = false;
function showObj(obj_id) {
	if (!inTree) {
		inTree =  true;
		$("#obj_detail").hide();
		$("#scan_detail").hide();
		$("#vuln_detail").hide();
		if (obj_id > 0) {
			$("#obj_detail").show();
			try {
				obj = top.getScanObject(selectedTask, obj_id);
				$("#obj_url").text(obj[2]);
				$("#obj_ref").text(obj[9]);
				$("#obj_code").text(obj[4]);
				$("#obj_size").text(obj[5]);
				$("#obj_type").text(obj[7]);
			}
			catch(e) {
				__alert("Can't get scan information:\n"+e);
			}
		}
		else {
			// overall information
			$("#scan_detail").show();
			try {
				obj = top.getTask(selectedTask);
				objs = top.getTaskObjects(selectedTask);
				vulns = top.getTaskVulners(selectedTask);
				$("#iscanStart").text(obj[4]);
				$("#iscanStop").text(obj[5]);
				$("#iscanPing").text(obj[6]);
				var sd = boostToDate(obj[4]);
				var fd = boostToDate(obj[5]);
				var pd = boostToDate(obj[6]);
				if (pd > fd) {
					fd = pd;
				}
				if (fd > sd) {
					sd = fd - sd;
					sd = sd / 1000;
					sd = sd.toFixed(0);
					slen = makeDuration(sd);
					if (obj[9] > 0) {
						sp = obj[9] / sd;
						sp = sp.toFixed(3);
						sp += " requests per second";
					}
					else {
						sp = "undefined";
					}
				}
				else {
					slen = "undefined";
					sp = "undefined";
				}
				$("#irqCount").text(obj[9]);
				$("#iscanLen").text(slen);
				$("#iscCount").text(objs.length);
				$("#ivlCount").text(vulns.length);
				$("#iscanSpeed").text(sp);
			}
			catch(e) {
				__alert("Can't get scan details:\n"+e);
			}
		}
	}
	if (obj_id == 0) {
		inTree = false;
	}
}

function showVuln(vuln_id) {
	if (!inTree) {
		inTree =  true;
		$("#obj_detail").hide();
		$("#scan_detail").hide();
		$("#vuln_detail").show();
		try {
			vuln = top.getVulner(selectedTask, vuln_id);
			if (vuln[1] > 0) { 
				obj = top.getScanObject(selectedTask, vuln[1]);
				url = obj[2];
			}
			else {
				objs = top.getTaskObjects(selectedTask);
				url = objs[0][2];
				url = url.replace(/[^:\/]+:\/\/[^\/]+(\/.*)/, "$1");
			}
			$("#vuln_title").text(vuln[6]);
			$("#vuln_severity").text(vuln[3]);
			$("#vuln_object").text(url);
			str = vuln[5];
			str = str.replace(/\</g, "&lt;");
			str = str.replace(/\>/g, "&gt;");
			$("#vuln_params").html("<code class='prettyprint'>" +str+ "</code></td>");
			prettyPrint();
		}
		catch(e) {
			__alert("Can't get vulnerability details:\n"+e);
		}
	}
}

function changeTarget() {
	var trgt = $("#task_target").val();
	var name = $("#task_name").val();
	if (name.match(/^Crawling\s+.*/) || name == "") {
		name = "Crawling " + trgt;
		$("#task_name").val(name);
	}
	return false;
}
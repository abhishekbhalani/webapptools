function showInfo(tsk_id)
{
	$("#mainView").load('/taskInfo.php', {'tsk_id': tsk_id});
	return false;
}

function RefreshList()
{
	$("#mainView").load('/tasks.php');
	return false;
}

function stopTaskOk(tsk_id)
{
	var dialog_buttons = {};
	dialog_buttons[btnOk] = function() {
						$(this).dialog('close');
					};
	$('#task_notify').load('taskActions.php', {'action': 'stop', 'task': tsk_id},
		function(resp, status, xreq) {
			if (status != "success") {
				$("#task_notify_text").html(xreq.status + " " + xreq.statusText);
				$("#task_notify").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
			else if (resp != "OK") {
				$("#task_notify_text").html(resp);
				$("#task_notify").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
		});
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
		buttons: dialog_buttons
	});
	return false;
}

function deleteTaskOk(tsk_id)
{
	var dialog_buttons = {};
	dialog_buttons[btnOk] = function() {
						$(this).dialog('close');
					};
	$('#task_notify').load('taskActions.php', {'action': 'del', 'task': tsk_id},
		function(resp, status, xreq) {
			if (status != "success") {
				$("#task_notify_text").html(xreq.status + " " + xreq.statusText);
				$("#task_notify").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
			else if (resp != "OK") {
				$("#task_notify_text").html(resp);
				$("#task_notify").dialog({
					autoOpen: true,
					width: 400,
					modal: true,
					buttons: dialog_buttons
				});
			}
			else {
				RefreshList();
			}
		});
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
		buttons: dialog_buttons
	});
	return false;
}

function pauseTask(tsk_id)
{
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
	}
	else {
		$('#task_filter').addClass("ui-state-highlight");
	}
}

function getCookie(c_name)
{
if (document.cookie.length>0)
  {
  c_start=document.cookie.indexOf(c_name + "=");
  if (c_start!=-1)
    {
    c_start=c_start + c_name.length+1;
    c_end=document.cookie.indexOf(";",c_start);
    if (c_end==-1) c_end=document.cookie.length;
    return unescape(document.cookie.substring(c_start,c_end));
    }
  }
return "";
}

function setCookie(c_name,value,expiredays)
{
var exdate=new Date();
exdate.setDate(exdate.getDate()+expiredays);
document.cookie=c_name+ "=" +escape(value)+
((expiredays==null) ? "" : ";expires="+exdate.toUTCString());
}

function applyFilter()
{
	$("input:[name=use_filter]").val(use_filter);
	var str = $("#filter_form").serializeArray();
	str = JSON.stringify(str);
	str = btoa(str);
	setCookie("WATTASKFILTER", str);
	RefreshList();
	return false;
}

function getFilter()
{
	var fltSetting = getCookie("WATTASKFILTER");
	use_filter = "0";
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
				use_filter = $("input:[name=use_filter]").val();
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
		buttons: dialog_buttons
	}); 
	return false;
}

function addTask()
{
	alert("Add new task");
}

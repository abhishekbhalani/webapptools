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

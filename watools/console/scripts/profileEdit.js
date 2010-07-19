function toggleShow(view)
{
	v = $('#ui_'+view).attr('visible');
	if (v == 0) {
		$('#ui_'+view).attr('visible', '1');
		$('#ui_'+view).show();
		$('#btn_'+view).removeClass('ui-icon-plus');
		$('#btn_'+view).addClass('ui-icon-minus');
	}
	else {
		$('#ui_'+view).attr('visible', '0');
		$('#ui_'+view).hide();
		$('#btn_'+view).removeClass('ui-icon-minus');
		$('#btn_'+view).addClass('ui-icon-plus');
	}
	return false;
}

function toggleShow2(view)
{
	$("#settings_view div[id^='sets_']").each(function (idx, domElem) {
		id = domElem.id;
		$("#"+id).hide();
	});
	$("#sets_"+view).show();
	return false;
}

function saveProfile()
{
	$("#pluginList").val('');
	var plg_txt = "";
	$("input:checkbox[id^='plg_']").each(function (idx, domElem) {
		idn = domElem.id;
		if (domElem.checked) {
			idn = idn.replace(/^plg_/, '');
			plg_txt += idn + ";";
		}
	});
	$("#plugin_list").val(plg_txt);
	plg_txt = "";
	$("[data_type]").each(function (idx, domElem) {
		idn = domElem.name;
		idt = $("input:[name="+idn+"]").attr('data_type');
		if (idt == '') {
			idt = 4;
		}
		plg_txt += idn + "=" + idt + ";";
	});
	$("#data_types").val(plg_txt);
	var settings = $("#profileForm").serialize();
	var dialog_buttons = {};
	dialog_buttons[btnOk] = function() {
							$(this).dialog('close');
						};
	$.post('profEdit.php', settings,
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
		});
	return false;
}

function setControlValue(nm, val) {
	var type = $('input:[name='+nm+']').attr('type');
	var value = unescape(val);
	if (type != undefined) {
		if (type == 'checkbox') {
			if (val != '' && val != '0') {
				$("input:[name="+nm+"]").attr('checked', 'checked');
			}
		}
		else if (type == 'radio') {
			var $radios = $('input:radio[name='+nm+']');
			$radios.filter('[value='+value+']').attr('checked', true);
		}
		else {
			$("input:[name="+nm+"]").val(value);
		}
	}
}

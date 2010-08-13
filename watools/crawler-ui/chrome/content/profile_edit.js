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
	$("#plugin_settings_title").html($('#sets_'+view+'_title').html());
	$('#sets_'+view+'_code').show();
	return false;
}

function saveProfile()
{
	$("#plugin_list").val('');
	var plg_txt = "";
	$("input:checkbox[id^='plg_']").each(function (idx, domElem) {
		idn = domElem.id;
		if (domElem.checked) {
			idn = idn.replace(/^plg_/, '');
			plg_txt += idn + ";";
		}
	});
	$("#plugin_list").val(plg_txt);
	types = new Array();
	$("[data_type]").each(function (idx, domElem) {
		idn = domElem.name;
		idt = $("input:[name="+idn+"]").attr('data_type');
		if (idt == '') {
			idt = 4;
		}
		types[idn] = idt;
	});
	$("#data_types").val("");

	$("#profile_editor input").each(function (idx, domElem) {
		var vn = domElem.name;
		var vv = domElem.value;
		var vt = domElem.type;
		tp = types[vn];
		if (tp == undefined || tp == "") {
			tp = 4;
		}
		if (vt == "radio") {
			if (!domElem.checked) {
				vn = ""; // not need to save
			}
		}
		if (vt == "checkbox") {
			if (tp == 2 && domElem.checked) {
				vv = 1;
			}
			else {
				vv = 0;
			}
		}
//alert(vt + "(" + selectedProfile + "): " + vn + ", " + tp + ", " + vv);
		if (vn != 'action' && vn != 'prof_id' && vn != 'data_types' && vn != "") {
			top.setProfileVar(selectedProfile, vn, tp, vv);
		}
	});
	drawProfiles();
	return false;
}

function setControlValue(nm, val) {
	var type = $('[name='+nm+']').attr('type');
	var value = unescape(val);
	if (type == undefined) {
		type = $('[name='+nm+']').attr('nodeName');
	}
	if (type != undefined) {
		if (type == 'checkbox') {
			if (val != '' && val != '0') {
				$("[name="+nm+"]").attr('checked', 'checked');
			}
		}
//		else if (type == 'textarea') {
//			$("[name="+nm+"]").attr('textContent', value);
//		}
		else if (type == 'radio') {
			var $radios = $('[name='+nm+']');
			$radios.filter('[value='+value+']').attr('checked', true);
		}
		else {
			$("[name="+nm+"]").val(value);
		}
	}
}

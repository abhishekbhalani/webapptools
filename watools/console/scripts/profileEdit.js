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

function saveProfile()
{
}

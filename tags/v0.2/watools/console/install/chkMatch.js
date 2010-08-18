function chkMatch(txt1, txt2, msg1, msg2, place)
{
	var st1 = $(txt1).val();
	var st2 = $(txt2).val();
	if (st1 != "" && st2 != "") {
		$(place).css('display', 'block');
		if (st1 == st2) {
			//alert(msg1);
			$(place).text(msg1);
			$(place).removeClass('ui-state-error');
			$(place).addClass('ui-state-highlight');
			return true;
		}
		else {
			//alert(msg2);
			$(place).text(msg2);
			$(place).removeClass('ui-state-highlight');
			$(place).addClass('ui-state-error');
			return true;
		}
	}
	else {
		//alert('All blank!');
		$(place).css('display', 'none');
	}
}
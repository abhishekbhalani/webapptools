function db_change(block) {
	$('fieldset').each( function(index) {
		//$(this).addClass('ui-state-disabled');
		$(this).children('table').addClass('ui-state-disabled');
		$(this).children('table').find('*').attr('disabled', true);
	});
	$(block).children('table').removeClass('ui-state-disabled');
	$(block).children('table').find('*').removeAttr('disabled');
}

function doStage1()
{
    var block = document.getElementById("checkResults");
    errs = 0;
    
    if (document.forms[0].redis_host.value == "") {
        var info = document.getElementById("srv");
        info.style.display = "list-item";
        errs = errs + 1;
    }
    else {
        var info = document.getElementById("srv");
        info.style.display = "none";
    }
    
    iv = parseInt(document.forms[0].redis_port.value);
    if (isNaN(iv) || iv < 1 || iv > 65535) {
        var info = document.getElementById("port");
        info.style.display = "list-item";
        errs = errs + 1;
    }
    else {
        var info = document.getElementById("port");
        info.style.display = "none";
    }
    
    iv = parseInt(document.forms[0].redis_db.value);
    if (isNaN(iv) || iv < 0 || iv > 15) {
        var info = document.getElementById("dbnum");
        info.style.display = "list-item";
        errs = errs + 1;
    }
    else {
        var info = document.getElementById("dbnum");
        info.style.display = "none";
    }
    
    // perform Redis connection check
    if (errs == 0) {
        var msg = "";
        var info = document.getElementById("redis");
        var rerr = document.getElementById("redis_err");
        
        var oRequest;
        try {
                oRequest=new XMLHttpRequest();
        } catch (e)   {
                try {
                        oRequest=new ActiveXObject("Msxml2.XMLHTTP");
                } catch (e) {
                        try {
                                oRequest=new ActiveXObject("Microsoft.XMLHTTP");
                        } catch (e) {
                                rerr.innerHTML = "YOUR BROWSER DOESN'T SUPPORT AJAX!";
                                return false;
                        }
                }
        }
        oRequest.open("GET", "stage1redis.php", false);
        oRequest.send(null);
        msg = oRequest.responseText;
        rerr.innerHTML = msg;
        if (msg != "SUCCESS") {
            info.style.display = "list-item";
            errs = errs + 1;
        }
        else {
            info.style.display = "none";
        }
    }

    if (errs > 0) {
        block.style.display = "block";
        return false;
        
    }
    return true;    
}
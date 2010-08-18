function CheckUser(usr, pwd) {
    user = escape(usr);
    pswd = escape(pwd);
    params = "uname=" + user + "&paswd=" + pswd + "&ajax=true";

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
                return "YOUR BROWSER DOESN'T SUPPORT AJAX!";
            }
        }
    }
    oRequest.open("POST", "/login.php", false);
    oRequest.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    oRequest.send(params);
    
	msg = oRequest.responseText;
	msg = jQuery.trim(msg);
    return msg;
}

<html>
<head>
<title>Web "A" Tools</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link type="text/css" href="/[{$theme}]/wat.css" rel="stylesheet" />	
<script type="text/javascript" src="/scripts/jquery.js"></script>
<script type="text/javascript" src="/scripts/jquery-ui.js"></script>
<script language="JavaScript" src="/scripts/chkUser.js"></script>
<script type="text/javascript">
    $(function(){
        // Dialog
        $('#dialog').dialog({
            autoOpen: true,
            draggable: false,
            resizable: false,
            closeOnEscape:false,
            modal: false,
            width: 350,
            open: function(event, ui) { $(".ui-dialog-titlebar-close").hide(); },
            buttons: {
                "Вход": function() {
                    var name = $("#uname").attr('value');
                    var pwd = $("#paswd").attr('value');
                    var resp = CheckUser(name, pwd);
                    if (resp == 'SUCCESS') {
                        $('#tips')
                            .text('')
                            .hide();
                            document.location = "index.php";
                    }
                    else {
                        $('#tips')
                            .html('<span style="float: left; margin-right: 0.3em;" class="ui-icon ui-icon-alert"></span>' + resp)
                            .show();
                        setTimeout(function() {
                            $('#tips').fadeOut("slow");
                        }, 2000);
                    }
                }
            }
        });
        $('#tips').hide();
    });
</script>
</head>
<body>
<div id="dialog" title="Вход в систему">
    <table>
        <tr>
            <td rowspan="3" valign="top"><img src="/[{$theme}]/images/appIcon.png" border="0"></td>
            <td>Имя пользователя</td>
            <td><input type="text" id="uname" name="uname"></td>
        </tr>
        <tr>
            <td>Пароль</td>
            <td><input type="password" id="paswd" name="paswd"></td>
        </tr>
        <tr>
            <td colspan="2"><p id="tips" class="ui-state-error ui-corner-all">Все поля должны быть заполнены.</p></td>
        </tr>
    </table>
</div>
</body>
</html>

<script>
var tabPersIndex = 2;
$(function() {
    $('#mainView button').each(function(){
        $(this).button({
            icons: {primary: 'ui-icon-check'}
            })
    });
    $('#instWarn').dialog({
        autoOpen: false,
        width: 400,
        modal: true,
        buttons: {
            'Ok': function() {
                $(this).dialog('close');
            }
        }
    });
    $('#themeWarn').dialog({
        autoOpen: false,
        width: 400,
        modal: true,
        buttons: {
            'Ok': function() {
                $(this).dialog('close');
            }
        }
    });
});
function SaveTheme() {
    var th = $("#theme").attr('value');
    var ln = $("#lang").attr('value');
    if (th == 0) {
        $('#instWarn').dialog('open');
    }
    else {
        $('#themeWarnMsg').html('');
        $('#themeWarnMsg').load('themesModyfy.php', {'object': 'themeusr', 'theme': th, 'lang': ln },
                function(data) {
                    resp = jQuery.trim(data);
                    if (resp == "") {
                        $('#themeWarnMsg').html("Unknown error");
                    }
                    if (resp != "OK") {
                        $('#themeWarn').dialog('open');
                    }
                    else {
                        //refresh page: $("#setPage").load("setsUsers.php");
                        document.location = '/';
                    }
                });
    }
}
function SaveData() {
    var desc = $("#userDesc").attr('value');
    var opwd = $("#userPwd").attr('value');
    var pwd1 = $("#userPwdNew").attr('value');
    var pwd2 = $("#userPwdCnf").attr('value');
    if (opwd == "" && (pwd1 != "" || pwd2 != "")) {
        $('#instWarnMsg').text('You must enter your current password to change it!');
        $('#instWarn').dialog('open');
        return false;
    }
    if (pwd1 != "" && pwd1 != pwd2) {
        $('#instWarnMsg').text('New password doesn\'t match the confirmation!');
        $('#instWarn').dialog('open');
        return false;
    }
    $('#themeWarnMsg').html('');
    $('#themeWarnMsg').load('uagModify.php', {'object': 'userself', 'desc': desc, 'opwd': opwd, 'pwd1': pwd1, 'pwd2': pwd2 },
        function(data) {
            resp = jQuery.trim(data);
            if (resp == "") {
                $('#themeWarnMsg').html("Unknown error");
            }
            if (resp != "OK") {
                $('#themeWarn').dialog('open');
            }
            else {
                //refresh page: $("#setPage").load("setsUsers.php");
            }
        });
}
</script>
<fieldset class="ui-widget ui-corner-all" width="100%">
<legend class="ui-widget"><span class="ui-icon ui-icon-person ui-icon-btn"></span> Учетная запись</legend>
<table border='0' class="data">
<tr><td><b>Имя пользователя:</b></td><td><input type="text" value="[{$UserName}]" readonly="1"></td></tr>
<tr><td><b>Полное имя:</b></td><td><input type="text" id="userDesc" name="udesc" value="[{$userName}]" class="text"></td></tr>
<tr><td><b>Текущий пароль:</b></td><td><input type="password" id="userPwd" name="upwd" value="" class="text"></td></tr>
<tr><td><b>Новый пароль:</b></td><td><input type="password" id="userPwdNew" name="upwdn" value="" class="text"></td></tr>
<tr><td><b>Подтверждение пароля:</b></td><td><input type="password" id="userPwdCnf" name="upwdc" value="" class="text"></td></tr>
<tr><td><b>&nbsp;</b></td><td><button id="accSettings" onClick="SaveData();">Применить</button></td></tr>
</table>
</fieldset>
<br/>
<fieldset class="ui-widget ui-corner-all" width="100%">
<legend class="ui-widget"><span class="ui-icon ui-icon-computer ui-icon-btn"></span> Настройки отображения</legend>
<table border='0' class="data">
<tr><td><b>Тема:</b></td><td><select id="theme">[{html_options values=$themes output=$thNames selected=$defTheme}]</select></td></tr>
<tr><td><b>Язык:</b></td><td><select id="lang">[{html_options values=$langs output=$lnNames selected=$defLang}]</select></td></tr>
<tr><td><b>&nbsp;</b></td><td><button id="guiSettings" onClick="SaveTheme();">Применить</button></td></tr>
</table>
</fieldset>
<br/>
<div id="instWarn" title='Warning'>
    <table><tr>
    <td><img src="/[{$theme}]/images/warning.png"</td>
    <td><p id='instWarnMsg'>Вы должны сперва инсталировать эту тему!</p></td>
    </tr></table>
</div>
<div id='themeWarn' title='Warning' class='ui-state-error'>
    <table class='ui-state-error' width='100%'><tr>
    <td><img src="/[{$theme}]/images/warning.png"</td>
    <td><p id='themeWarnMsg' width='100%'>-</p></td>
    </tr></table>
</div>
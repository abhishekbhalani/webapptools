<script>
var tabUsrIndex = 3;
var user = {
    uid: 0,
    login: '',
    desc: '',
    pwd1: '',
    pwd2: '',
    grp: 0,
    setID: function(sval) {
        $('#uID').value = sval;
        this.uid = sval;
    },
    setLogin: function(sval) {
        alert("Login: " + sval);
        $('#uLogin').val(sval);
        this.login = sval;
    },
    setDesc: function(sval) {
        $('#uDesc').val(sval);
        this.desc = sval;
    },
    setPwd1: function(sval) {
        $('#uPwd').val(sval);
        this.pwd1 = sval;
    },
    setPwd2: function(sval) {
        $('#uPwd2').val(sval);
        this.pwd2 = sval;
    },
    setGroup: function(sval) {
        $('#uGroup').val(sval);
        this.grp = sval;
    }
};
var group = {
    gid: 0,
    name: '',
    desc: '',
    setID: function(sval) {
        $('#gID').val(sval);
        this.gid = sval;
    },
    setName: function(sval) {
        $('#gName').val(sval);
        this.name = sval;
    },
    setDesc: function(sval) {
        $('#gDesc').val(sval);
        this.desc = sval;
    },
};

$(function() {
    $('#users-all').button({
        icons: {primary: 'ui-icon-person'}
    });
    $('#users-none').button({
        icons: {primary: 'ui-icon-cancel'}
    });
    $('#users-invert').button({
        icons: {primary: 'ui-icon-refresh'}
    });
    $('#groups-all').button({
        icons: {primary: 'ui-icon-person'}
    });
    $('#groups-none').button({
        icons: {primary: 'ui-icon-cancel'}
    });
    $('#groups-invert').button({
        icons: {primary: 'ui-icon-refresh'}
    });
    $('#groupEdit').dialog({
        autoOpen: false,
        width: 400,
        modal: true,
        buttons: {
            'Ok': function() {
                $(this).dialog('close');
                groupBoxClick('ok');
            },
            'Cancel': function() {
                $(this).dialog('close');
            }
        }
    });
    $('#errorBox').dialog({
        autoOpen: false,
        width: 400,
        modal: true,
        buttons: {
            'Close': function() {
                $(this).dialog('close');
            }
        }
    });
    // prevent to show dialog layout
    $('#confirmBox').dialog({
        autoOpen: false
    });
});

function CustomConfirm(text, action) {
    $('#confirmBox').dialog({
        autoOpen: false,
        height: 300,
        width: 400,
        modal: true,
        buttons: {
            'Да': function() {
                $(this).dialog('close');
                action();
            },
            'Нет': function() {
                $(this).dialog('close');
            }
        }
    });
    $('#confirmMessageText').html(text);
    $('#confirmBox').dialog('open');
}
function SelectUsers (id) {
    $('#'+id).find('input').attr('checked', true);
}
function UnselectUsers (id) {
    $('#'+id).find('input').attr('checked', false);
}
function InvertUsers (id) {
    var bx = $('#'+id).find('input');
    bx.each(function(idx,elem) {
        if(elem.checked) {
            elem.checked = false;
        }
        else {
            elem.checked = true;
        }
    });
}
function groupAdd() {
    $("#ui-dialog-title-groupEdit").text('Создать группу');
    group.setName('');
    //$('#gName').attr('value', '');
    group.setDesc('');
    //$('#gDesc').attr('value', '');
    group.setID('-1');
    //$('#gID').attr('value', '-1');
    $('#groupEdit').dialog('open');
    return false;
}
function groupEdit(gname, gdesc, gid) {
    $("#ui-dialog-title-groupEdit").text('Редактировать группу'); 
    group.setName(gname);
    //$('#gName').attr('value', gname);
    group.setDesc(gdesc);
    //$('#gDesc').attr('value', gdesc);
    group.setID(gid);
    //$('#gID').attr('value', gid);
    $('#groupEdit').dialog('open');
    return false;
}
function groupBoxClick(status) {
    if (status == 'ok') {
        var gid = group.gid; //$('#gID').attr('value');
        var gnm = group.name; //$('#gName').attr('value');
        var gds = group.desc; //$('#gDesc').attr('value');
        $('#errorMessageText').html('');
        $('#errorMessageText').load('uagModify.php', {'object': 'group', 'gid': gid, 'gname': gnm, 'gdesc': gds},
                function(data) {
                    resp = jQuery.trim(data);
                    if (resp == "") {
                        $('#errorMessageText').html("Неизвестная ошибка");
                    }
                    if (resp != "OK") {
                        $('#errorBox').dialog('open');
                    }
                    else {
                        $("#tabs").tabs( 'load' , tabUsrIndex );
                        //refresh page: $("#setPage").load("setsUsers.php");
                    }
                });
    }
    return false;
}
function userAdd() {
    $("#ui-dialog-title-userEdit").text('Добавить пользователя'); 
    //$('#userEdit').data('title.dialog', 'Add new user'); 
    user.setLogin('');
    //$('#uLogin').val(uname);
    user.setDesc('');
    //$('#uDesc').val(udesc);
    user.setID('-1');
    //$('#uID').val(uid);
    user.setPwd1('');
    //$('#uPwd').attr('value', '');
    user.setPwd2('');
    //$('#uPwd2').attr('value', '');
    $('#uLogin').attr('class', 'text');
    $('#uLogin').attr('readonly', false);
    $('#uDesc').attr('class', 'text');
    $('#uDesc').attr('readonly', false);
    $('#pwd1row').css('display','table-row');
    $('#pwd2row').css('display','table-row');
    $('#userEdit').dialog({
        autoOpen: true,
        width: 400,
        modal: true,
        buttons: {
            'Ok': function() {
                userBoxClick('ok'); // , $('#uID').val(), $('#uLogin').val(), $('#uDesc').val(), $('#uPwd').val(), $('#uPwd2').val()
                $(this).dialog('close');
            },
            'Cancel': function() {
                $(this).dialog('close');
            }
        }
    });
    return false;
}
function userEdit(uname, udesc, uid) {
    $("#ui-dialog-title-userEdit").text('Редактировать пользователя'); 
    user.setLogin(uname);
    //$('#uLogin').val(uname);
    user.setDesc(udesc);
    //$('#uDesc').val(udesc);
    user.setID(uid);
    //$('#uID').val(uid);
    user.setPwd1('');
    //$('#uPwd').attr('value', '');
    user.setPwd2('');
    //$('#uPwd2').attr('value', '');
    $('#uLogin').attr('class', 'text');
    $('#uLogin').attr('readonly', false);
    $('#uDesc').attr('class', 'text');
    $('#uDesc').attr('readonly', false);
    $('#pwd1row').css('display','none');
    $('#pwd2row').css('display','none');
    $('#userEdit').dialog({
        autoOpen: true,
        width: 400,
        modal: true,
        buttons: {
            'Ok': function() {
                userBoxClick('ok'); // , $('#uID').val(), $('#uLogin').val(), $('#uDesc').val(), $('#uPwd').val(), $('#uPwd2').val()
                $(this).dialog('close');
            },
            'Отмена': function() {
                $(this).dialog('close');
            }
        }
    });
    return false;
}
function userPwd(uname, udesc, uid) {
    $("#ui-dialog-title-userEdit").text('Сбросить пароль');
    user.setLogin(uname);
    //$('#uLogin').val(uname);
    user.setDesc(udesc);
    //$('#uDesc').val(udesc);
    user.setID(uid);
    //$('#uID').val(uid);
    user.setPwd1('');
    //$('#uPwd').attr('value', '');
    user.setPwd2('');
    //$('#uPwd2').attr('value', '');
    $('#uLogin').attr('class', 'readonly');
    $('#uLogin').attr('readonly', true);
    $('#uDesc').attr('class', 'readonly');
    $('#uDesc').attr('readonly', true);
    $('#pwd1row').css('display','table-row');
    $('#pwd2row').css('display','table-row');
    var state = false;
    $('#userEdit').dialog({
        autoOpen: true,
        width: 400,
        modal: true,
        buttons: {
            'Ok': function() {
                $(this).dialog('close');
                state = true;
                alert ("UserClick: " + state + ": " + user.pwd1);
                userBoxClick('ok'); // , $('#uID').val(), $('#uLogin').val(), $('#uDesc').val(), $('#uPwd').val(), $('#uPwd2').val()
            },
            'Отмена': function() {
                $(this).dialog('close');
            }
        }
    });
    return false;
}
function userBoxClick(status) { // , mid, muname, mdesc, mpw, mpw2
    if (status == 'ok') {
        var gid = user.uid; //$('#uID').val();
        var gnm = user.login; // $('#uLogin').val()
        var gds = user.desc; // $('#uDesc').val()
        var gpw = user.pwd1; //$('#uPwd').val();
        var gp2 = user.pwd2; //$('#uPwd2').val();
        var grp = user.grp;
        
        alert(gid + ", " + gnm + ", " + gds + ", " + gpw + "(" + gp2 + "), " + grp);
        $('#errorMessageText').html('');
        if (gpw != gp2) {
            $('#errorMessageText').html("Пароли не совпадают!");
            $('#errorBox').dialog('open');
        }
        else if (gnm == "") {
            $('#errorMessageText').html("Имя пользователя пустое!");
            $('#errorBox').dialog('open');
        }
        else {
            $('#errorMessageText').load('uagModify.php', {'object': 'user', 'uid': gid, 'uname': gnm, 'udesc': gds, 'pwd': gpw, 'grp': grp},
                function(data) {
                    resp = jQuery.trim(data);
                    if (resp == "") {
                        $('#errorMessageText').html("Неизвестная ошибка");
                    }
                    if (resp != "OK") {
                        $('#errorBox').dialog('open');
                    }
                    else {
                        alert("Success!");
                        $("#tabs").tabs( 'load' , tabUsrIndex );
                        //refresh page: $("#setPage").load("setsUsers.php");
                    }
                });
        }
    }
    return false;
}
function deleteUser(id) {
    $('#errorMessageText').html("Удалить пользователя: " + id);
    $('#errorMessageText').load('uagModify.php', {'object': 'userdel', 'uid': id},
                function(data) {
                    resp = jQuery.trim(data);
                    if (resp == "") {
                        $('#errorMessageText').html("Неизвестная ошибка");
                    }
                    if (resp != "OK") {
                        $('#errorBox').dialog('open');
                    }
                    else {
                        $("#tabs").tabs( 'load' , tabUsrIndex );
                        //refresh page: $("#setPage").load("setsUsers.php");
                    }
                });
    return false;
}
function deleteUserMsg(uid, uname) {
    CustomConfirm("Вы действительно хотите удалить пользователя '" + uname + "'?", function(){deleteUser(uid);});
    return false;
}
function deleteGroup(id) {
    $('#errorMessageText').html("Удалить группу: " + id);
    $('#errorMessageText').load('uagModify.php', {'object': 'groupdel', 'gid': id},
                function(data) {
                    resp = jQuery.trim(data);
                    if (resp == "") {
                        $('#errorMessageText').html("Неизвестная ошибка");
                    }
                    if (resp != "OK") {
                        $('#errorBox').dialog('open');
                    }
                    else {
                        $("#tabs").tabs( 'load' , tabUsrIndex );
                        //refresh page: $("#setPage").load("setsUsers.php");
                    }
                });
    return false;
}
function deleteGroupMsg(uid, uname) {
    CustomConfirm("Вы действительно хотите удалить группу '" + uname + "'?", function(){deleteGroup(uid);});
    return false;
}
</script>
<table width="100%"><tr valign="top">
    <td widht="50%"><fieldset class="fields">
        <legend class="fields"><span class="ui-icon ui-icon-person ui-icon-btn"></span> Пользователи</legend>
        <a href="#" id="user-add" onclick="return userAdd();"><span class="ui-icon ui-icon-plusthick ui-icon-btn" title="Добавить пользователя"></span></a>
        <a href="#" id="user-del" onclick="return false;"><span class="ui-icon ui-icon-closethick ui-icon-btn" title="Удалить выбранных пользователей"></span></a>
        <br/>
        <table width="100%" id='userstable'>
        <tr class="ui-widget-header"><th class="grid">Имя пользователя</th><th class="grid">Полное имя</th><th class="grid">Группы</th><th class="grid">Действия</th></tr>
        [{section name=user loop=$userList}]
        <tr><td class="grid"><input type="checkbox" name="uid[{$userList[user][0]}]" id="uid[{$userList[user][0]}]">[{$userList[user][1]}]</td><td class="grid">[{$userList[user][2]}]</td>
                <td class="grid">[{section name=grpN loop=$userList[user][10]}]
                    [{$userList[user][10][grpN]}][{if not $smarty.section.grpN.last}], [{/if}]
                [{/section}]</td><td class="grid">
                                <a href="#" onclick="return deleteUserMsg('[{$userList[user][0]}]', '[{$userList[user][1]}]');">
                                    <span class="ui-icon ui-icon-closethick ui-icon-btn" title="Удалить пользователя"></span></a>&nbsp;
                                <a href="#" onclick="return userEdit('[{$userList[user][1]}]', '[{$userList[user][2]}]', '[{$userList[user][0]}]');">
                                    <span class="ui-icon ui-icon-clipboard ui-icon-btn" title="Редактировать пользователя"></span></a>&nbsp;
                                <a href="#" onclick="return userPwd('[{$userList[user][1]}]', '[{$userList[user][2]}]', '[{$userList[user][0]}]');">
                                    <span class="ui-icon ui-icon-key-export ui-icon-btn" title="Сбросить пароль"></span></a>&nbsp;
                                <a href="#" onclick="return false;">
                                    <span class="ui-icon ui-icon-group ui-icon-btn" title="Определить группы"></span></a></td></tr>
        [{/section}]
        </table>
        <br/>
        <span><button id="users-all" onclick="SelectUsers('userstable'); return false;">Выбрать всех</button></span>
        <span><button id="users-none" onclick="UnselectUsers('userstable'); return false;">Сбросить выделение</button></span>
        <span><button id="users-invert" onclick="InvertUsers('userstable'); return false;">Инвертировать выделение</button></span>
        </fieldset>
    </td><td><fieldset class="fields">
        <legend class="fields"><span class="ui-icon ui-icon-group ui-icon-btn"></span>Группы</legend>
        <a href="#" id="group-add" onclick="return groupAdd();"><span class="ui-icon ui-icon-plusthick ui-icon-btn" title="Добавить группу"></span></a>
        <a href="#" id="group-del" onclick="return false;"><span class="ui-icon ui-icon-closethick ui-icon-btn" title="Удалить выбранные группы"></span></a>
        <br/>
        <table width="100%" id='groupstable'>
        <tr class="ui-widget-header"><th class="grid">Название</th><th class="grid">Описание</th><th class="grid">Действия</th></tr>
        [{section name=group loop=$groupList}]
        <tr><td class="grid"><input type="checkbox" name="gid[{$groupList[group][0]}]" id="gid[{$groupList[group][0]}]">[{$groupList[group][1]}]</td><td class="grid">[{$groupList[group][2]}]</td><td class="grid">
                                <a href="#" onclick="return deleteGroupMsg('[{$groupList[group][0]}]', '[{$groupList[group][1]}]');">
                                    <span class="ui-icon ui-icon-closethick ui-icon-btn" title="Удалить группу"></span></a>&nbsp;
                                <a href="#" onclick="return groupEdit('[{$groupList[group][1]}]', '[{$groupList[group][2]}]', '[{$groupList[group][0]}]')">
                                    <span class="ui-icon ui-icon-clipboard ui-icon-btn" title="Редактировать группу"></span></a></td></tr>
        [{/section}]
        </td></tr>
        </table>
        <br/>
        <span><button id="groups-all" onclick="SelectUsers('groupstable'); return false;">Выбрать все</button></span>
        <span><button id="groups-none" onclick="UnselectUsers('groupstable'); return false;">Сбросить выделение</button></span>
        <span><button id="groups-invert" onclick="InvertUsers('groupstable'); return false;">Инвертировать выделение</button></span>
        </fieldset>
    </td>
</tr></table>
<!-- group edit -->
<div id="groupEdit" title="Редактировать группу" style="display: none;">
    <table width="100%">
        <tr><td><b>Название</b></td><td><input type="text" id="gName" value="" class="text" onchange="group.name=this.value; return false;"></td></tr>
        <tr><td><b>Описание</b></td><td><input type="text" id="gDesc" value="" class="text" onchange="group.desc=this.value; return false;"></td></tr>
    </table>
    <input type="hidden" id="gID" name="gID" value="">
</div>
<!-- user edit -->
<div id="userEdit" title="Редактировать пользователя" style="display: none;">
    <table width="100%">
        <tr><td><b>Имя пользователя</b></td><td><input type="text" id="uLogin" name="uLogin" value="" class="text ui-widget-content ui-corner-all" onchange="user.login=this.value; return false;"></td></tr>
        <tr><td><b>Полное имя</b></td><td><input type="text" id="uDesc" name="uDesc" value="" class="text ui-widget-content ui-corner-all" onchange="user.desc=this.value; return false;"></td></tr>
        <tr id="pwd1row"><td><b>Пароль</b></td><td><input type="password" id="uPwd" name="uPwd" value="" class="text ui-widget-content ui-corner-all" onchange="user.pwd1=this.value; return false;"></td></tr>
        <tr id="pwd2row"><td><b>Подтверждение</b></td><td><input type="password" id="uPwd2" name="uPwd2" value="" class="text ui-widget-content ui-corner-all" onchange="user.pwd2=this.value; return false;"></td></tr>
        <tr><td><b>Группа</b></td><td><select id="uGroup" name="uGroup" class="ui-widget-content ui-corner-all" onchange="user.grp=this.value; return false;">
                [{html_options values=$grpIds output=$grpNames}]
                </select></td></tr>
    </table>
    <input type="hidden" name="uID" id="uID" value="">
</div>
<!-- error box -->
<div id="errorBox" title="Ошибка!" class='ui-state-error'>
    <div class='ui-state-error ui-conner-all'>
    <img src="/[{$theme}]/images/exit.png" align="left"><p id="errorMessageText">-</p>
    </div>
</div>
<!-- confirm box -->
<div id="confirmBox" title="Подтверждение">
    <img src="/[{$theme}]/img/warning.png" align="left"><p id="confirmMessageText">-</p>
</div>

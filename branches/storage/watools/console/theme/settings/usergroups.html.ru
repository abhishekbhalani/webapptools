<script>
var tabUsrIndex = 3;
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
            'Отмена': function() {
                $(this).dialog('close');
            }
        }
    });
    $('#userEdit').dialog({
        autoOpen: false,
        width: 400,
        modal: true,
        buttons: {
            'Ok': function() {
                $(this).dialog('close');
                userBoxClick('ok');
            },
            'Отмена': function() {
                $(this).dialog('close');
            }
        }
    });
    $('#errorBox').dialog({
        autoOpen: false,
        width: 400,
        modal: true,
        buttons: {
            'Закрыть': function() {
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
    $('#gName').attr('value', '');
    $('#gDesc').attr('value', '');
    $('#gID').attr('value', '-1');
    $('#groupEdit').dialog('open');
    return false;
}
function groupEdit(gname, gdesc, gid) {
    $("#ui-dialog-title-groupEdit").text('Редактировать группу'); 
    $('#gName').attr('value', gname);
    $('#gDesc').attr('value', gdesc);
    $('#gID').attr('value', gid);
    $('#groupEdit').dialog('open');
    return false;
}
function groupBoxClick(status) {
    if (status == 'ok') {
        gid = $('#gID').attr('value');
        gnm = $('#gName').attr('value');
        gds = $('#gDesc').attr('value');
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
    $("#ui-dialog-title-userEdit").text('Создать пользователя'); 
    //$('#userEdit').data('title.dialog', 'Add new user'); 
    $('#uLogin').attr('value', '');
    $('#uDesc').attr('value', '');
    $('#uID').attr('value', '-1');
    $('#uPwd').attr('value', '');
    $('#uPwd2').attr('value', '');
    $('#uLogin').attr('class', 'text');
    $('#uLogin').attr('readonly', false);
    $('#uDesc').attr('class', 'text');
    $('#uDesc').attr('readonly', false);
    $('#pwd1row').css('display','table-row');
    $('#pwd2row').css('display','table-row');
    $('#userEdit').dialog('open');
    return false;
}
function userEdit(uname, udesc, uid) {
    $("#ui-dialog-title-userEdit").text('Редактировать пользователя'); 
    $('#uLogin').attr('value', uname);
    $('#uDesc').attr('value', udesc);
    $('#uID').attr('value', uid);
    $('#uPwd').attr('value', '');
    $('#uPwd2').attr('value', '');
    $('#uLogin').attr('class', 'text');
    $('#uLogin').attr('readonly', false);
    $('#uDesc').attr('class', 'text');
    $('#uDesc').attr('readonly', false);
    $('#pwd1row').css('display','none');
    $('#pwd2row').css('display','none');
    $('#userEdit').dialog('open');
    return false;
}
function userPwd(uname, udesc, uid) {
    $("#ui-dialog-title-userEdit").text('Задать пароль'); 
    $('#uLogin').attr('value', uname);
    $('#uDesc').attr('value', udesc);
    $('#uID').attr('value', uid);
    $('#uPwd').attr('value', '');
    $('#uPwd2').attr('value', '');
    $('#uLogin').attr('class', 'readonly');
    $('#uLogin').attr('readonly', true);
    $('#uDesc').attr('class', 'readonly');
    $('#uDesc').attr('readonly', true);
    $('#pwd1row').css('display','table-row');
    $('#pwd2row').css('display','table-row');
    $('#userEdit').dialog('open');
    return false;
}
function userBoxClick(status) {
    if (status == 'ok') {
        gid = $('#uID').attr('value');
        gnm = $('#uLogin').attr('value');
        gds = $('#uDesc').attr('value');
        gpw = $('#uPwd').attr('value');
        gp2 = $('#uPwd2').attr('value');
        grp = $('#uGroup').attr('value');
        
        $('#errorMessageText').html('');
        if (gpw != gp2) {
            $('#errorMessageText').html("Пароли не совпадают!");
        }
        else if (gnm == "") {
            $('#errorMessageText').html("Имя учетной записи пустое!");
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
    $('#errorMessageText').html("Удаление пользователя: " + id);
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
    $('#errorMessageText').html("Удаление группы: " + id);
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
        <a href="#" id="user-add" onclick="return userAdd();"><span class="ui-icon ui-icon-plusthick ui-icon-btn"></span></a>
        <a href="#" id="user-del" onclick="return false;"><span class="ui-icon ui-icon-closethick ui-icon-btn"></span></a>
        <br/>
        <table width="100%" id='userstable'>
        <tr class="ui-widget-header"><th class="grid">Логин</th><th class="grid">Польное имя</th><th class="grid">Группы</th><th class="grid">Действия</th></tr>
        [{section name=user loop=$userList}]
        <tr><td class="grid"><input type="checkbox" name="uid[{$userList[user][9]}]" id="uid[{$userList[user][9]}]">[{$userList[user][0]}]</td><td class="grid">[{$userList[user][1]}]</td>
                <td class="grid">[{section name=grpN loop=$userList[user][10]}]
                    [{$userList[user][10][grpN]}][{if not $smarty.section.grpN.last}], [{/if}]
                [{/section}]</td><td class="grid">
                                <a href="#" onclick="return deleteUserMsg('[{$userList[user][9]}]', '[{$userList[user][0]}]');">
                                    <span class="ui-icon ui-icon-closethick ui-icon-btn" title="удалить"></span></a>&nbsp;
                                <a href="#" onclick="return userEdit('[{$userList[user][0]}]', '[{$userList[user][1]}]', '[{$userList[user][9]}]');">
                                    <span class="ui-icon ui-icon-clipboard ui-icon-btn" title="редактировать"></span></a>&nbsp;
                                <a href="#" onclick="return userPwd('[{$userList[user][0]}]', '[{$userList[user][1]}]', '[{$userList[user][9]}]');">
                                    <span class="ui-icon ui-icon-key-export ui-icon-btn" title="задать пароль"></span></a>&nbsp;
                                <a href="#" onclick="return false;">
                                    <span class="ui-icon ui-icon-group ui-icon-btn" title="определить группы"></span></a></td></tr>
        [{/section}]
        </table>
        <br/>
        <span><button id="users-all" onclick="SelectUsers('userstable'); return false;">Выбрать всех</button></span>
        <span><button id="users-none" onclick="UnselectUsers('userstable'); return false;">Сбросить выбор</button></span>
        <span><button id="users-invert" onclick="InvertUsers('userstable'); return false;">Инвертировать выбор</button></span>
        </fieldset>
    </td><td><fieldset class="fields">
        <legend class="fields"><span class="ui-icon ui-icon-group ui-icon-btn"></span>Группы</legend>
        <a href="#" id="group-add" onclick="return groupAdd();"><span class="ui-icon ui-icon-plusthick ui-icon-btn"></span></a>
        <a href="#" id="group-del" onclick="return false;"><span class="ui-icon ui-icon-closethick ui-icon-btn"></span></a>
        <br/>
        <table width="100%" id='groupstable'>
        <tr class="ui-widget-header"><th class="grid">Название</th><th class="grid">Описание</th><th class="grid">Действия</th></tr>
        [{section name=group loop=$groupList}]
        <tr><td class="grid"><input type="checkbox" name="gid[{$groupList[group][9]}]" id="gid[{$groupList[group][9]}]">[{$groupList[group][0]}]</td><td class="grid">[{$groupList[group][1]}]</td><td class="grid">
                                <a href="#" onclick="return deleteGroupMsg('[{$groupList[group][9]}]', '[{$groupList[group][0]}]');">
                                    <span class="ui-icon ui-icon-closethick ui-icon-btn" title="удалить"></span></a>&nbsp;
                                <a href="#" onclick="return groupEdit('[{$groupList[group][0]}]', '[{$groupList[group][1]}]', '[{$groupList[group][9]}]')">
                                    <span class="ui-icon ui-icon-clipboard ui-icon-btn" title="редактировать"></span></a></td></tr>
        [{/section}]
        </td></tr>
        </table>
        <br/>
        <span><button id="groups-all" onclick="SelectUsers('groupstable'); return false;">Выбрать все</button></span>
        <span><button id="groups-none" onclick="UnselectUsers('groupstable'); return false;">Сбросить выбор</button></span>
        <span><button id="groups-invert" onclick="InvertUsers('groupstable'); return false;">Инвертировать выбор</button></span>
        </fieldset>
    </td>
</tr></table>
<!-- popup windows -->
<div id="blanket" style='display:none;'></div>
<!-- group edit -->
<div id="groupEdit" title="Редактировать группу">
    <table width="100%">
        <tr><td><b>Название группы</b></td><td><input type="text" id="gName" value="" class="text"></td></tr>
        <tr><td><b>Описание группы</b></td><td><input type="text" id="gDesc" value="" class="text"></td></tr>
    </table>
    <input type="hidden" id="gID" name="gID" value="">
</div>
<!-- user edit -->
<div id="userEdit" title="Edit User">
    <table width="100%">
        <tr><td><b>Логин</b></td><td><input type="text" id="uLogin" value="" class="text"></td></tr>
        <tr><td><b>Полное имя</b></td><td><input type="text" id="uDesc" value="" class="text"></td></tr>
        <tr id="pwd1row"><td><b>Пароль</b></td><td><input type="password" id="uPwd" value="" class="text"></td></tr>
        <tr id="pwd2row"><td><b>Подтверждение</b></td><td><input type="password" id="uPwd2" value="" class="text"></td></tr>
        <tr><td><b>Основная группа</b></td><td><select id="uGroup" name="uGroup">
                [{html_options values=$grpIds output=$grpNames}]
                </select></td></tr>
    </table>
    <input type="hidden" name="uID" id="uID" value="">
</div>
<!-- error box -->
<div id="errorBox" title="Ошибка!" class='ui-state-error'>
    <img src="/[{$theme}]/img/exit.png" align="left"><p id="errorMessageText">-</p>
</div>
<!-- confirm box -->
<div id="confirmBox" title="Подтверждение">
    <img src="/[{$theme}]/img/warning.png" align="left"><p id="confirmMessageText">-</p>
</div>

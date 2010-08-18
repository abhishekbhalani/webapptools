<html>
<head>
<title>Web "A" Tools</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" type="text/css" href="/[{$theme}]/wat.css" />
<script language="JavaScript" src="scripts/jquery.js"></script>
<script type="text/javascript" src="/scripts/jquery-ui.js"></script>
</head>
<body>
<div id="content">
<table width="100%" border="0"><tr>
    <td class="button-panel" valign="top" width="150px">
        <button id="btnTestRefresh" onClick="document.location='/';">Пользователь [{$UserName}]</button>
        <script>
        $("#btnTestRefresh").button({icons: {primary: 'ui-icon-refresh'}}).width("150px");
        </script>
        <ol id="main-menu" class="leftMenu">
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/task.png" border="0" align="middle"/>Задачи</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/scanner.png" border="0" align="middle" />Профили сканирования</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/report.png" border="0" align="middle" />Отчеты</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/dictionary.png" border="0" align="middle"/>Справочники</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/bug.png" border="0" align="middle"/>Уязвимости</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/settings.png" border="0" align="middle" />Настройки</li>
            <li class="ui-widget-content ui-state-default ui-corner-all">
                <img src="/[{$theme}]/images/exit.png" border="0" align="middle" />Выход</li>
        </ol>
    </td>
<td valign="top">
<div width="100%" height="100%" class="ui-widget-content" id="mainView"></div>
</td>
</tr></table>
</div>
<script language="JavaScript">
$(document).ready(function() {
    // load the dashboard
    $("#mainView").load("dashboard.php");
});
</script>
<script language="JavaScript" src="/scripts/mainmenu.js"></script>
</body>
</html>

<html>
<head>
    <title>Установка Web "A" Tools</title>
	<link type="text/css" href="/theme/wat.css" rel="stylesheet" />	
	<script type="text/javascript" src="/scripts/jquery.js"></script>
	<script type="text/javascript" src="/scripts/jquery-ui.js"></script>
    <script type="text/javascript" src="stage1.js"></script>
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
                "Далее >": function() {
					$('#form1').submit();
                }
            }
        });
        $('#tips').hide();
		$('#db_redis').attr('checked', 'checked');
		db_change('#redis');
    });
	</script>
</head>
<body background="/theme/images/ui-bg_diagonals-thick_90_eeeeee_40x40.png">
	<div id="dialog" title="Step 1. Database connection.">
    <p>Настройка соединения с БД для этого сервера</p>
    <form action="stage2.php" method="POST" onsubmit="return doStage1();" id="form1">
        <fieldset width="0" id="redis">
            <legend><b><input type="radio" name="db_type" id="db_redis" value="redis" checked onchange="db_change('#redis');">Конфигурация REDIS DB</b></legend>
            <table border="0">
                <tr><td>Адрес:</td><td><input type="text" name="redis_host" value="127.0.0.1"></td></tr>
                <tr><td>Порт:</td><td><input type="text" name="redis_port" value="6379"></td></tr>
                <tr><td>Ауфентикация:</td><td><input id="redis_pass" type="password" name="redis_pass" value=""></td></tr>
                <tr><td>База данных:</td><td><input type="text" name="redis_db" value="0"></td></tr>
                <tr><td>Очистить БД:</td><td><input type="checkbox" name="redis_clear" id="redis_clear" value="0">
                                            <label for="redis_clear">Да</label></td></tr>
            </table>
        </fieldset>
        <fieldset width="0" id="sqlite">
            <legend><b><input type="radio" name="db_type" id="db_sqlite" value="sqlite" onchange="db_change('#sqlite');">Конфигурация Sqlite3</b></legend>
            <table border="0">
                <tr><td>Путь к БД:</td><td><input type="text" name="sqlite_path" value="/tmp/watools.sqlite"></td></tr>
                <tr><td>Очистить БД:</td><td><input type="checkbox" name="redis_clear" id="redis_clear" value="0">
                                            <label for="sqlite_clear">Да</label></td></tr>
            </table>
        </fieldset>
        <br/>
        <div id="checkResults" style="display:none">
            <fieldset width="0">
                <legend><b>Ошибки конфигурации</b></legend>
                <ul>
                <li id="srv" class="error">Server host must not be empty!</li>
                <li id="port" class="error">Server port is not valid!</li>
                <li id="dbnum" class="error">Database number is not valid!</li>
                <li id="redis" class="error">Redis error: <span id="redis_err"></span>!</li>
                </ul>
            </fieldset>
            <br/>
        </div>
    </form>
	</div>
</body>
</html>
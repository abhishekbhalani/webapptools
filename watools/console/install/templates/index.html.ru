<html>
<head>
    <title>Установка Web "A" Tools</title>
	<link type="text/css" href="/theme/wat.css" rel="stylesheet" />	
	<script type="text/javascript" src="/scripts/jquery.js"></script>
	<script type="text/javascript" src="/scripts/jquery-ui.js"></script>
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
    });
	</script>
</head>
<body background="/theme/images/ui-bg_diagonals-thick_90_eeeeee_40x40.png">
<div id="dialog" title="Добро пожаловать">
    <div>Этот мастер проведет Вас через процесс установки.</div>
    <form action="stage1.php" method="POST" id="form1">
	Выберите язык <select name="lang">[{html_options values=$langs output=$lnNames selected=$defLang}]</select>
    </form>
</body>
</html>
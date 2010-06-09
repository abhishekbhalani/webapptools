<h3 class="ui-widget-header-h3">Настройки</h3>
<script language="JavaScript">
    $(function() {
        $("#tabs").tabs({
            ajaxOptions: {
                error: function(xhr, status, index, anchor) {
                    $(anchor.hash).html("Невозможно загрузить эту страницу. Мы исправим эту ситуацию максимально быстро.");
                }
            }
        });
    });
</script>
<div id="tabs">
    <ul>
        <li><a href="setsPersonal.php">Персональные настройки</a></li>
        <li><a href="setsModules.php">Модули</a></li>
[{if $SysUser}]
        <li><a href="setsSystem.php">Системные настройки</a></li>
        <li><a href="setsUsers.php">Пользователи и группы</a></li>
        <li><a href="setsACL.php">Контроль доступа</a></li>
        <li><a href="setsDatabase.php">Базы данных</a></li>
[{/if}]
    </ul>
</div>

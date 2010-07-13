<h3 class="ui-widget-header-h3">Обзор</h3>
[{if $SysUser === 18 }]
<fieldset class="ui-widget">
<legend class="ui-widget">Сервер</legend>
<table border='0' class="data">
<tr><td><img src="[{$theme}]/img/users_total.png" align="left"><b>Пользователей в системе:</b></td><td>[{$sysUsers}]</td></tr>
<tr><td><img src="[{$theme}]/img/users_online.png" align="left"><b>Пользователей on-line:</b></td><td>[{$sysUsersOnline}]</td></tr>
<tr><td><img src="[{$theme}]/img/tasks_active.png" align="left"><b>Активных задач:</b></td><td>[{$actTasks}]</td></tr>
<tr><td><img src="[{$theme}]/img/scanners_active.png" align="left"><b>Активных сканеров:</b></td><td>2</td></tr>
<tr><td><img src="[{$theme}]/img/reports_base.png" align="left"><b>Отчетов в базе:</b></td><td>30</td></tr>
<tr><td><img src="[{$theme}]/img/reports_gens.png" align="left"><b>Отчетов генерируется:</b></td><td>3</td></tr>
</table>
</fieldset>
<br/>
[{/if}]
[{if $SysUser}]
<fieldset class="ui-widget ui-corner-all">
<legend class="ui-widget">Сервер Базы Данных</legend>
<table border='0'>
<tr><td><span class="ui-icon ui-icon-computer ui-icon-btn"></span><b> Сервер БД:</b></td><td>[{$sqlDriver}]</td></tr>
<tr><td><span class="ui-icon ui-icon-info ui-icon-btn"></span><b> Версия сервера:</b></td><td>[{$sqlVersion}]</td></tr>
</table>
</fieldset>
<br/>
<fieldset class="ui-widget ui-corner-all">
<legend class="ui-widget">Сервер приложения</legend>
<table border='0' class="data">
<tr><td><span class="ui-icon ui-icon-group ui-icon-btn"></span><b> Пользователей в системе:</b></td><td>[{$sysUsers}]</td></tr>
<tr><td><span class="ui-icon ui-icon-person ui-icon-btn"></span><b> Пользователей on-line:</b></td><td>[{$sysUsersOnline}]</td></tr>
<tr><td><span class="ui-icon ui-icon-lightbulb ui-icon-btn"></span><b> Активных сессий:</b></td><td>[{$sysSessions}]</td></tr>
</table>
</fieldset>
<br/>
[{/if}]
<fieldset class="ui-widget ui-corner-all">
<legend class="ui-widget">Система</legend>
<table border='0'>
<tr><td><span class="ui-icon ui-icon-computer ui-icon-btn"></span><b> Операционная система:</b></td><td>[{$osName}]</td></tr>
<tr><td><span class="ui-icon ui-icon-cpu ui-icon-btn"></span><b> Загрузка CPU:</b></td><td>[{$cpuStat}]</td></tr>
<tr><td><span class="ui-icon ui-icon-compress ui-icon-btn"></span><b> Загрузка RAM:</b></td><td>[{$ramStat}]</td></tr>
<tr><td><span class="ui-icon ui-icon-disk ui-icon-btn"></span><b> Загрузка диска:</b></td><td>[{$dskStat}]</td></tr>
</table>
</fieldset>
<br/>
<fieldset class="ui-widget ui-corner-all">
<legend class="ui-widget">Пользовательские данные</legend>
<table border='0'>
<tr><td colspan="2"><!-- img src="[{$theme}]/img/sql_info.png" align="left" --><b> Имя пользователя [{$UserDesc}]</b></td></tr>
<tr><td><span class="ui-icon ui-icon-gear ui-icon-btn"></span><b> Активных задач:</b></td><td>[{$actTasks}]</td></tr>
<tr><td><span class="ui-icon ui-icon-gear ui-icon-btn"></span><b> Завершенных задач:</b></td><td>[{$complTasks}]</td></tr>
<tr><td><span class="ui-icon ui-icon-print ui-icon-btn"></span><b> Отчетов генерируется:</b></td><td>3</td></tr>
</table>
</fieldset>
<br/>
</div>

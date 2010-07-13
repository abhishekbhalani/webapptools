<script language="javascript">
function showInfo(tsk_id)
{
	$("#mainView").load('/taskInfo.php', {'tsk_id': tsk_id});
}
</script>
<h3 class="ui-widget-header-h3">Задачи</h3>
<div style="margin: 20;">
[{if $TaskNum > 0}]
	<table width="100%" id='userstable'>
		<tr class="ui-widget-header">
			<th style="width: auto;">Профиль</th>
			<th style="width: auto;">Цель</th>
			<th style="width: 5%;">Статус</th>
			<th style="width: 5%;">Завершение</th>
			<th style="width: auto;">Время начала</th>
			<th style="width: auto;">Время завершения</th>
			<th style="width: auto;">Действия</th>
		</tr>
[{section name=task loop=$TaskList}]<tr>
			<td style="white-space: nowrap; text-align: center;">[{$TaskList[task][1]}]</td>
			<td style="width: auto; text-align: center;">[{$TaskList[task][6]}]</td>
			<td style="width: 5%; text-align: center;">[{$TaskList[task][2]}]</td>
			<td style="width: 5%; text-align: center;">[{$TaskList[task][3]}]%</td>
			<td style="width: auto; text-align: center;">[{$TaskList[task][4]}]</td>
			<td style="width: auto; text-align: center;">[{$TaskList[task][5]}]</td>
			<td style="white-space: nowrap">
				<a href="#" onclick="return showInfo('[{$TaskList[task][0]}]');">
					<span class="ui-icon ui-icon-document ui-icon-btn" title="Детальная информация"></span></a>
[{if $TaskACL == 1}]
				<a href="#" onclick="return stopTask('[{$TaskList[task][0]}]');">
					<span class="ui-icon ui-icon-circle-close ui-icon-btn" title="Остановить задачу"></span></a>
				<a href="#" onclick="return deleteTask('[{$TaskList[task][0]}]');">
					<span class="ui-icon ui-icon-closethick ui-icon-btn" title="Удалить задачу"></span></a>
[{/if}]
                </td>
		</tr>
            [{/section}]
	</table>
[{else}]
	<div class="ui-state-error ui-corner-all" width="100%" style="padding: 5px;">
		<span style="float: left; margin-right: 0.3em;" class="ui-icon ui-icon-alert"></span>
		Не найдено задач!
	</div>
[{/if}]
</div>
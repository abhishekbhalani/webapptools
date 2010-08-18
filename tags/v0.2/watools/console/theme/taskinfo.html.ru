<script language="javascript">
function goBack()
{
	$("#mainView").load('/tasks.php');
	return false;
}
function toggleView(view)
{
	v = $('#'+view).attr('visible');
	if (v == 0) {
		$('#'+view).attr('visible', '1');
		$('#'+view).show();
		$('#'+view+"_btn").removeClass('ui-icon-plus');
		$('#'+view+"_btn").addClass('ui-icon-minus');
	}
	else {
		$('#'+view).attr('visible', '0');
		$('#'+view).hide();
		$('#'+view+"_btn").removeClass('ui-icon-minus');
		$('#'+view+"_btn").addClass('ui-icon-plus');
	}
	return false;
}
</script>
<h3 class="ui-widget-header-h3">Task - [{$target}] (started at [{$startTime}])</h3>
<div style="margin: 20;">
<a href="#" onclick="return goBack();"><span class="ui-icon ui-icon-circle-arrow-n ui-icon-btn" title="к списку задач"></span>Вернуться к списку задач</a><br/>
<h4 class="ui-widget-header"><a href="#" onclick="return toggleView('summary');">
	<span class="ui-icon ui-icon-minus ui-icon-btn" id="summary_btn" title="Objects"></span></a>Summary</h4>
	<table width="100%" id='summary' visible="1">
	<tr><td style="white-space: nowrap; text-align: left;width: 50;"><b>Начало сканирования:</b></td><td>[{$scanStart}]</td></tr>
	<tr><td style="white-space: nowrap; text-align: left;width: 50;"><b>Завершение сканирования:</b></td><td>[{$scanStop}]</td></tr>
	<tr><td style="white-space: nowrap; text-align: left;width: 50;"><b>Активность:</b></td><td>[{$scanPing}]</td></tr>
	<tr><td style="white-space: nowrap; text-align: left;width: 50;"><b>Продолжительность:</b></td><td>[{$scanLen}]</td></tr>
	<tr><td style="white-space: nowrap; text-align: left;width: 50;"><b>Найдено объектов:</b></td><td>[{$scCount}]</td></tr>
	<tr><td style="white-space: nowrap; text-align: left;width: 50;"><b>Найдено уязвимостей:</b></td><td>[{$vlCount}]</td></tr>
	<tr><td style="white-space: nowrap; text-align: left;width: 50;"><b>Скорость сканирования:</b></td><td>[{$scanSpeed}]</td></tr>
	</table>
<br/>
<h4 class="ui-widget-header"><a href="#" onclick="return toggleView('objects');">
	<span class="ui-icon ui-icon-plus ui-icon-btn" id="objects_btn" title="Объекты"></span></a>Найденные объекты ([{$scCount}])</h4>
[{if $scCount > 0}]
	<table width="100%" id='objects' style="display: none;" visible="0">
		<tr class="ui-widget-header">
			<th style="width: 5%;">ID</th>
			<th style="width: auto;">URL</th>
			<th style="width: auto;">Код ответа</th>
			<th style="width: auto;">Размер данных</th>
			<th style="width: auto;">Тип содержимого</th>
		</tr>
[{section name=sc loop=$scdata}]<tr>
			<td style="white-space: nowrap; text-align: center;">[{$scdata[sc][0]}]</td>
			<td style="white-space: nowrap; text-align: left;">[{$scdata[sc][3]}]</td>
			<td style="width: auto; text-align: center;">[{$scdata[sc][5]}]</td>
			<td style="width: auto; text-align: center;">[{$scdata[sc][6]}]</td>
			<td style="width: auto; text-align: center;">[{$scdata[sc][8]}]</td>
		</tr>
            [{/section}]
	</table>
[{else}]
	<div class="ui-state-error ui-corner-all" width="100%" id='objects' style="padding: 5px;" visible="1">
		<span style="float: left; margin-right: 0.3em;" class="ui-icon ui-icon-alert"></span>
		Не найдено ни одного объекта!
	</div>
[{/if}]
<br/>
<h4 class="ui-widget-header"><a href="#" onclick="return toggleView('vulners');">
	<span class="ui-icon ui-icon-plus ui-icon-btn" id="vulners_btn" title="Уязвимости"></span></a>Найденые уязвимости ([{$vlCount}])</h4>
[{if $vlCount > 0}]
	<table width="100%" id='vulners' style="display: none;" visible="0">
		<tr class="ui-widget-header">
			<th style="width: 5%;">ID объекта</th>
			<th style="width: 15%;">Код</th>
			<th style="width: auto;">Заголовок</th>
			<th style="width: 5%;">Критичность</th>
			<th style="width: auto;">Параметры</th>
		</tr>
[{section name=vl loop=$vldata}]<tr>
			<td style="white-space: nowrap; text-align: left;">[{$vldata[vl][0]}]</td>
			<td style="width: auto; text-align: center;">[{$vldata[vl][1]}]</td>
			<td style="width: auto; text-align: center;">[{$vldata[vl][2]}]</td>
			<td style="width: auto; text-align: center;">[{$vldata[vl][3]}]</td>
			<td style="width: auto; text-align: center;">[{$vldata[vl][4]}]</td>
		</tr>
            [{/section}]
	</table>
[{else}]
	<div class="ui-state-error ui-corner-all" width="100%" id='vulners' style="padding: 5px;" visible="1">
		<span style="float: left; margin-right: 0.3em;" class="ui-icon ui-icon-alert"></span>
		Не найдено ни одной уязвимости!
	</div>
[{/if}]
<br/>
<a href="#" onclick="return goBack();"><span class="ui-icon ui-icon-circle-arrow-n ui-icon-btn" title="к списку задач"></span>Вернуться к списку задач</a><br/>
</div>
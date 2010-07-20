<div class="ui-widget">
    <script src="/scripts/modules.js"></script>
    <!-- pre>Debug:[{$debug}]</pre -->
    <fieldset class="fields">
        <legend class="fields"><span class="ui-icon ui-icon-gear ui-icon-btn"></span> Сканирующие модули</legend>
[{if $ScanNum > 0}]
        <table width="100%" id='userstable'>
            <tr class="ui-widget-header">
                <th style="width: auto;">Экземпляр</th>
                <th style="width: auto;">Версия</th>
                <th style="width: auto;">Адрес</th>
                <th style="width: auto;">Название</th>
                <th style="width: auto;">Задач</th>
                <th style="width: auto;">Статус</th>
                <th style="width: auto;">Действия</th>
            </tr>
[{section name=scanner loop=$ScanModules}]<tr>
                <td width="10%" style="white-space: nowrap">[{$ScanModules[scanner][0]}]</td>
                <td style="width: auto; text-align: center;">[{$ScanModules[scanner][1]}]</td>
                <td style="width: auto;">[{$ScanModules[scanner][2]}]</td>
                <td style="width: auto;">[{$ScanModules[scanner][3]}]</td>
                <td style="width: auto; text-align: center;">[{$ScanModules[scanner][4]}]</td>
                <td style="width: auto; text-align: center;">[{$ScanModules[scanner][5]}]</td>
                <td style="white-space: nowrap">
                    <a href="#" onclick="return showInfo('scanner', '[{$ScanModules[scanner][0]}]');">
                        <span class="ui-icon ui-icon-info ui-icon-btn" title="Информация о сканере"></span></a>
[{if $ScanACL == 1}]
                    <a href="#" onclick="return stopModule('scanner', '[{$ScanModules[scanner][0]}]');">
                        <span class="ui-icon ui-icon-power ui-icon-btn" title="Остановить сканер"></span></a>
                    <a href="#" onclick="return restartModule('scanner', '[{$ScanModules[scanner][0]}]');">
                        <span class="ui-icon ui-icon-refresh ui-icon-btn" title="Перезапустить сканер"></span></a>
                    <a href="#" onclick="return cmdQueueEdit('scanner', '[{$ScanModules[scanner][0]}]');">
                        <span class="ui-icon ui-icon-wrench ui-icon-btn" title="Резактировать список команд"></span></a>
[{/if}]
                </td>
            </tr>
            [{/section}]
        </table>
        [{else}]
         <div class="ui-state-error ui-corner-all" width="100%" style="padding: 5px;">
            <span style="float: left; margin-right: 0.3em;" class="ui-icon ui-icon-alert"></span>
            Не найдено ни одного сканера!
         </div>
        [{/if}]
    </fieldset>
    <br/>
    <fieldset class="fields">
        <legend class="fields"><span class="ui-icon ui-icon-print ui-icon-btn"></span> Генераторы отчетов</legend>
        [{if ReportNum > 0}]
        [{else}]
         <div class="ui-state-error ui-corner-all" width="100%" style="padding: 5px;">
            <span style="float: left; margin-right: 0.3em;" class="ui-icon ui-icon-alert"></span>
            Не найдено ни одного генератора отчетов!
         </div>
        [{/if}]
    </fieldset>
</div>
<div id="moduleNotify" title="Информация о модуле"></div>
<div title="Подтверждение" id="moduleConfirm" style="display: none;">
    <table><tr>
        <td><!-- img src="/[{$theme}]/img/warning.png" --> </td><td>Вы действительно хотите <span id="confirmAction"></span> <span id="confirmClass"></span>
        <span id="confirmUuid"></span>?</td>
    </tr></table>
</div>
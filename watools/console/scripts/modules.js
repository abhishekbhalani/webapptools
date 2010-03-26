function showInfo(cls, uuid, inst)
{
    $('#moduleNotify').load('modActions.php', {'action': "info", 'class': cls, 'uuid': uuid, 'instance': inst},
            function(data) {
                resp = jQuery.trim(data);
                if (resp == "") {
                    $('#moduleNotify').html("Unknown error");
                }
                $('#moduleNotify').dialog({
                        autoOpen: true,
                        width: 400,
                        modal: true,
                        buttons: {
                            'Ok': function() {
                                $(this).dialog('close');
                            }
                        }                        
                    });
            });
    return false;
}

function stopModule(cls, uuid)
{
    $("#confirmAction").text('stop');
    $("#confirmClass").text(cls);
    $("#confirmUuid").text(uuid);
    $('#moduleConfirm').dialog({
            autoOpen: true,
            width: 400,
            modal: true,
            buttons: {
                'Yes': function() {
                    $(this).dialog('close');
                    $('#moduleNotify').load('modActions.php', {'action': 'stop', 'class': cls, 'uuid': uuid},
                            function(data) {
                                resp = jQuery.trim(data);
                                if (resp == "") {
                                    $('#moduleNotify').html("Unknown error");
                                }
                                $('#moduleNotify').dialog({
                                        autoOpen: true,
                                        width: 400,
                                        modal: true,
                                        buttons: {
                                            'Ok': function() {
                                                $(this).dialog('close');
                                            }
                                        }                        
                                    });
                            });
                },
                'No': function() {
                    $(this).dialog('close');
                }
            }
        });
    return false;
}

function restartModule(cls, uuid)
{
    $("#confirmAction").text('restart');
    $("#confirmClass").text(cls);
    $("#confirmUuid").text(uuid);
    $('#moduleConfirm').dialog({
            autoOpen: true,
            width: 400,
            modal: true,
            buttons: {
                'Yes': function() {
                    $(this).dialog('close');
                    $('#moduleNotify').load('modActions.php', {'action': 'restart', 'class': cls, 'uuid': uuid},
                            function(data) {
                                resp = jQuery.trim(data);
                                if (resp == "") {
                                    $('#moduleNotify').html("Unknown error");
                                }
                                $('#moduleNotify').dialog({
                                        autoOpen: true,
                                        width: 400,
                                        modal: true,
                                        buttons: {
                                            'Ok': function() {
                                                $(this).dialog('close');
                                            }
                                        }                        
                                    });
                            });
                },
                'No': function() {
                    $(this).dialog('close');
                }
            }
        });
    return false;
}

function cmdQueueEdit(cls, uuid)
{
    return false;
}

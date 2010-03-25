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

function stopModule(cls, uuid, inst)
{
    $("#confirmAction").text('stop');
    $("#confirmClass").text(cls);
    $("#confirmUuid").text(uuid);
    $("#confirmInst").text(inst);
    $('#moduleConfirm').dialog({
            autoOpen: true,
            width: 400,
            modal: true,
            buttons: {
                'Yes': function() {
                    $(this).dialog('close');
                    $('#moduleNotify').load('modActions.php', {'action': 'stop', 'class': cls, 'uuid': uuid, 'instance': inst},
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

function restartModule(cls, uuid, inst)
{
    $("#confirmAction").text('restart');
    $("#confirmClass").text(cls);
    $("#confirmUuid").text(uuid);
    $("#confirmInst").text(inst);
    $('#moduleConfirm').dialog({
            autoOpen: true,
            width: 400,
            modal: true,
            buttons: {
                'Yes': function() {
                    $(this).dialog('close');
                    $('#moduleNotify').load('modActions.php', {'action': 'restart', 'class': cls, 'uuid': uuid, 'instance': inst},
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

function cmdQueueEdit(cls, uuid, inst)
{
    return false;
}

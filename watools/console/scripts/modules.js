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

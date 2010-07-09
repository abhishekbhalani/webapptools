function db_change() {
	block = $('#db_drv').val();
	$('fieldset').each( function(index) {
		$(this).hide();
	});
	$('#' + block).show();
}

function doStage1()
{
	retval = true;
	db_dsn = "";
	db_prt = "";
	db_usr = "";
	db_pwd = "";
	db_db = "";
	db_clr = 0;
	db_drv = $('#db_drv').val();
	db_clr = $("#db_clear").attr('checked');

	if (db_drv == "SQlite3") {
		db_drv = "sqlite";
		db_dsn = $("#sqlite_path").val();
		db_usr = $("#sqlite_uname").val();
		db_pwd = $("#sqlite_passwd").val();
		if (db_dsn == "") {
			$('#dberr_msg').text("Select path to sqlite database file!");
			retval = false;
		}
	}
	else {
		if (db_drv == "MySQL") {
			db_drv = "mysql";
			db_dsn = $("#mysql_host").val();
			db_prt = $("#mysql_port").val();
			db_usr = $("#mysql_uname").val();
			db_pwd = $("#mysql_passwd").val();
			db_db = $("#mysql_db").val();
		}
		else if (db_drv == "MS-SQL") {
			db_drv = "dblib";
			db_dsn = $("#mssql_host").val();
			db_prt = $("#mssql_port").val();
			db_usr = $("#mssql_uname").val();
			db_pwd = $("#mssql_passwd").val();
			db_db = $("#mssql_db").val();
		}
		else if (db_drv == "Oracle") {
			db_drv = "oci";
			db_dsn = $("#oracle_host").val();
			db_prt = $("#oracle_port").val();
			db_usr = $("#oracle_uname").val();
			db_pwd = $("#oracle_passwd").val();
			db_db = $("#oracle_db").val();
		}
		else {
			$('#dberr_msg').text("Unsupported DB selected");
			$('#dberr').show();
			retval = false;
		}
		if (db_dsn == "") {
			$('#dbsrv').show();
			retval = false;
		}
		if (db_prt == "") {
			$('#dbport').show();
			retval = false;
		}
		if (db_db == "") {
			$('#dbname').show();
			retval = false;
		}
		db_dsn = "host=" + db_dsn + ";port=" + db_prt + ";dbname=" + db_db;
	}
	if (!retval) {
		$('#checkResults').show();
		$('#checkResultsFS').show();
	}
	else {
		db_query = {"dsn" : db_drv + ":" + db_dsn,
					"uname" : db_usr,
					"pswd" : db_pwd,
					"drop" : db_clr
					};
		alert("Data:" + 
			"\nDSN: " + db_query.dsn +
			"\nUser: " + db_query.uname +
			"\nPassword: " + db_query.pswd +
			"\nDrop: " + db_query.drop);
			
		$('#dberr_msg').html('');
		$('#dberr_msg').load('stage1check.php', db_query,
			function(data) {
				resp = jQuery.trim(data);
				if (resp == "") {
					$('#dberr_msg').html("Unknown error");
					$('#dberr').show();
					retval = false;
				}
				if (resp != "OK") {
					$('#db_dsn').val(db_query.dsn);
					$('#db_usr').val(db_query.uname);
					$('#db_pwd').val(db_query.pswd);
					retval = true;
				}
				else {
					$('#dberr').show();
					retval = false;
				}
			}
		);
	}
    return retval;    
}

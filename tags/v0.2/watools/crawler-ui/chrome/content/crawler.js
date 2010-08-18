/*const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;
const Cu = Components.utils;*/
const prefService = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefBranch);
const storageService = Cc["@mozilla.org/storage/service;1"].getService(Ci.mozIStorageService);
const nsIFilePicker = Components.interfaces.nsIFilePicker;
var dbFile = null;
var mDB = new SQLiteHandler();
var bSharedPagerCache = 2048;
var dbVersion = "";
var bConnected = false;

function __alert(txt, title) {
	cnt.__alert(txt, title);
}

function GetWD() {
	var file = Components.classes["@mozilla.org/file/directory_service;1"].
					getService(Components.interfaces.nsIProperties).
					get("CurProcD", Components.interfaces.nsIFile);
	return file.path;
}

function GetOS() {
	return osString = Components.classes["@mozilla.org/xre/app-info;1"]
					.getService(Components.interfaces.nsIXULRuntime).OS;
}

function setDatabase(nsiFileObj) {
	//try connecting to database
	bConnected = false;
	try  {
		if(nsiFileObj != null) {
			//create backup before opening
			/*this.createTimestampedBackup(nsiFileObj);

			var mi = $$("menu-general-sharedPagerCache");
			var bSharedPagerCache = mi.hasAttribute("checked");*/
			bConnected =  mDB.openDatabase(nsiFileObj, bSharedPagerCache);
			dbVersion = mDB.sqliteVersion;
			//__alert("SQLite version: " + dbVersion);
		}
	}
	catch (e)  {
		__alert("Connect to '" + nsiFileObj.path + "' failed: " + e);
		return;
	}
}

function GetDBVersion() {
	return dbVersion;
}

function openDatabaseWithPath(sPath) {
	var newfile = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
	try {
		newfile.initWithPath(sPath);
	} catch (e) {
		__alert(sm_getLStr("File not found: ") + sPath);
		//SmGlobals.mru.remove(sPath);
		return false;
	}
	if(newfile.exists()) {
		this.setDatabase(newfile);
		return true;
	}
	else {
		__alert(sm_getLStr("File not found: ") + sPath);
		// SmGlobals.mru.remove(sPath);
	}
	return false;
}

function getFile(sPath) {
	try {
		var f = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
		f.initWithPath(sPath);
		return f;
	} catch (e) {
		Cu.reportError('FileIO.getFile("' + sPath + '"): ' + e.message);
	}
	return null;
}

function openDatabase(fName, chSize) {        
	bSharedPagerCache = chSize;
	dbFile = getFile(fName);
	if (dbFile == null)
		return true;
		//check whether the file still exists
    var sPath = dbFile.path;
    if(!dbFile.exists()) {
		//__alert("File not found: " + sPath);
		try {
			dbFile.create(Ci.nsIFile.NORMAL_FILE_TYPE, 0644);
		}
		catch(e) {
			__alert("Can't create file " + sPath + "\n" + e.message);
			return true;
		}
		//this.closeDatabase(false);
		//SmGlobals.mru.remove(sPath);
		//this.setDatabase(null);
    }
    openDatabaseWithPath(sPath);
	return false;
}

function SaveTxtFile(text) {
	var fp = Components.classes["@mozilla.org/filepicker;1"]
				   .createInstance(nsIFilePicker);
	fp.init(window, "Save file", nsIFilePicker.modeSave);
	fp.appendFilters(nsIFilePicker.filterAll | nsIFilePicker.filterText);
	
	var rv = fp.show();
	if (rv == nsIFilePicker.returnOK || rv == nsIFilePicker.returnReplace) {
		var file = fp.file;
		
		if ( file.exists() == false ) {
			file.create( Components.interfaces.nsIFile.NORMAL_FILE_TYPE, 420 );
		}
		var outputStream = Components.classes["@mozilla.org/network/file-output-stream;1"]
			.createInstance( Components.interfaces.nsIFileOutputStream );
		/* Open flags 
		#define PR_RDONLY       0x01
		#define PR_WRONLY       0x02
		#define PR_RDWR         0x04
		#define PR_CREATE_FILE  0x08
		#define PR_APPEND      0x10
		#define PR_TRUNCATE     0x20
		#define PR_SYNC         0x40
		#define PR_EXCL         0x80
		*/
		/*
		** File modes ....
		**
		** CAVEAT: 'mode' is currently only applicable on UNIX platforms.
		** The 'mode' argument may be ignored by PR_Open on other platforms.
		**
		**   00400   Read by owner.
		**   00200   Write by owner.
		**   00100   Execute (search if a directory) by owner.
		**   00040   Read by group.
		**   00020   Write by group.
		**   00010   Execute by group.
		**   00004   Read by others.
		**   00002   Write by others
		**   00001   Execute by others.
		**
		*/
		outputStream.init( file, 0x04 | 0x08 | 0x20, 420, 0 );
		var result = outputStream.write( text, text.length );
		outputStream.close();
	}
}

function setPref(name, value) {
	try {
		prefService.setCharPref("extensions.crawler." + name, value);
	}
	catch(e) {
		__alert("Can't set preference:\nextensions.crawler." + name + " = " + value + "\n" + e);
	}
}

function getPref(name, defValue) {
	var p;
	if ( prefService.prefHasUserValue("extensions.crawler." + name) ) {
		p = prefService.getCharPref("extensions.crawler." + name);
	}
	else {
		p = defValue;
	}
	return p;
}

function runCrawler(exe, args) {
	var lFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
	var lPath = exe;
	//lPath = lPath.replace(/\\/g, "/");
	lFile.initWithPath(lPath);
	var process = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
	process.init(lFile);
	process.runAsync(args, args.length);
}

function getProfiles () {
	var __profs = new Array();
	if (bConnected) {
		mDB.selectQuery("SELECT profile_id, value FROM profile WHERE name='profile_name' ORDER BY value ASC");
		for (var i = 0; i < mDB.aTableData.length; i++) {
			__profs.push(new Array(mDB.aTableData[i][0], mDB.aTableData[i][1]));
		}
	}
	return __profs;
}

function getProfileVar (id, name) {
	var __profs;
	if (bConnected) {
		mDB.selectQuery("SELECT value FROM profile WHERE name='"+name+"' AND profile_id="+id);
		if (mDB.aTableData.length > 0) {
			__profs = mDB.aTableData[0][0];
		}
		else {
			__profs = undefined;
		}
	}
	return __profs;
}

function setProfileVar (id, name, type, value) {
	if (bConnected) {
		var params = new Array();
		params.push(new Array(0, id, SQLiteTypes.INTEGER));
		params.push(new Array(1, name, SQLiteTypes.TEXT));
		params.push(new Array(2, type, SQLiteTypes.INTEGER));
		params.push(new Array(3, value, SQLiteTypes.TEXT));
		mDB.executeWithParams("REPLACE INTO profile (profile_id,name,type,value) VALUES(?,?,?,?)", params);
	}
}

function getProfileValues (id) {
	var __profs = new Array();
	if (bConnected) {
		mDB.selectQuery("SELECT name, value FROM profile WHERE profile_id="+id);
		for (var i = 0; i < mDB.aTableData.length; i++) {
			__profs.push(new Array(mDB.aTableData[i][0], mDB.aTableData[i][1]));
		}
	}
	return __profs;
}

function profileDropBooleanValues(id) {
	if (bConnected) {
		qers = new Array();
		qers.push("UPDATE profile SET value=0 WHERE type=2 AND profile_id="+id);
		mDB.executeSimpleSQLs(qers);
	}
}

function getPluginList() {
	var __profs = new Array();
	if (bConnected) {
		mDB.selectQuery("SELECT plugin_id,plugin_name,ui_icon FROM profile_ui WHERE locale='en' ORDER BY plugin_name");
		for (var i = 0; i < mDB.aTableData.length; i++) {
			__profs.push(new Array(mDB.aTableData[i][0], mDB.aTableData[i][1], mDB.aTableData[i][2]));
		}
	}
	return __profs;
}

function getPluginUI(pID)
{
	var __profs = "";
	if (bConnected) {
		query = "SELECT ui_settings FROM profile_ui WHERE locale='en' AND plugin_id='" + pID + "'";
		mDB.selectQuery(query);
		if (mDB.aTableData.length > 0) {
			__profs = mDB.aTableData[0][0];
		}
	}
	return __profs;
}

function delProfileDB(pID) {
	var resp = "OK";
	if (bConnected) {
		try{
			mDB.selectQuery("DELETE FROM profile WHERE profile_id="+pID);
		}
		catch (e) {
			rest = "Delete profile failed: " + e;
		}
	}
	else {
		resp = "Not connected to DataBase!";
	}
	return resp;
}

function addProfile(name) {
	retval = "OK";
	
	if (bConnected) {
		name = name.replace(/\'/g, "''");
		req = "SELECT profile_id FROM profile WHERE name=='profile_name' AND value=='"+name+"'";
		mDB.selectQuery(req);
		if (mDB.aTableData.length == 0) {
			mDB.selectQuery("SELECT max(profile_id) FROM profile");
			if (mDB.aTableData.length > 0) {
				id = mDB.aTableData[0][0];
				id++;
				mDB.selectQuery("INSERT INTO profile (profile_id,name,type,value) VALUES("+id+",'profile_name',4,'"+name+"')");
			}
			else {
				retval = "Can't obtain profile number!";
			}
		}
		else {
			retval = "Profile with same name already exists: " + name;
		}
	}
	return retval;
}

function cloneProfile(src, name) {
	retval = "OK";
	
	if (bConnected) {
		name = name.replace(/\'/g, "''");
		req = "SELECT profile_id FROM profile WHERE name=='profile_name' AND value=='"+name+"'";
		mDB.selectQuery(req);
		if (mDB.aTableData.length == 0) {
			mDB.selectQuery("SELECT max(profile_id) FROM profile");
			if (mDB.aTableData.length > 0) {
				id = mDB.aTableData[0][0];
				id++;
				mDB.selectQuery("INSERT INTO profile (profile_id,name,type,value) VALUES("+id+",'profile_name',4,'"+name+"')");
				mDB.selectQuery("INSERT INTO profile (profile_id, name, type, value) SELECT "+id+" as profile_id, name, type, value FROM profile WHERE profile_id=="+src+" and name != 'profile_name'");
			}
			else {
				retval = "Can't obtain profile number!";
			}
		}
		else {
			retval = "Profile with same name already exists: " + name;
		}
	}
	return retval;
}

function getTaskList() {
	var __profs = new Array();
	if (bConnected) {
		mDB.selectQuery("SELECT id,profile_id,status,completion,start_time,finish_time,ping_time,processed_urls,name,requests  FROM task");
		for (var i = 0; i < mDB.aTableData.length; i++) {
			__profs.push(new Array(mDB.aTableData[i][0], mDB.aTableData[i][1], mDB.aTableData[i][2], mDB.aTableData[i][3], mDB.aTableData[i][4], mDB.aTableData[i][5], mDB.aTableData[i][6], mDB.aTableData[i][7], mDB.aTableData[i][8], mDB.aTableData[i][9]));
		}
		for (var i in __profs) {
			__profs[i] = fixTaskInfo(__profs[i]);
		}
	}
	return __profs;
}

function getTask(id) {
	var __profs = new Array();
	if (bConnected) {
		mDB.selectQuery("SELECT id,profile_id,status,completion,start_time,finish_time,ping_time,processed_urls,name,requests FROM task WHERE id="+id);
		if (mDB.aTableData.length > 0) {
			__profs = new Array(mDB.aTableData[0][0], mDB.aTableData[0][1], mDB.aTableData[0][2], mDB.aTableData[0][3], mDB.aTableData[0][4], mDB.aTableData[0][5], mDB.aTableData[0][6], mDB.aTableData[0][7], mDB.aTableData[0][8], mDB.aTableData[0][9]);
			__profs = fixTaskInfo(__profs);
		}
	}
	return __profs;
}

function deleteTask(id) {
	if (bConnected) {
		qers = new Array();
		qers.push("DELETE FROM task WHERE id=" + id);
		qers.push("DELETE FROM scan_data WHERE task_id=" + id);
		qers.push("DELETE FROM vulners WHERE task_id=" + id);
		mDB.executeSimpleSQLs(qers);
	}
}

function setTskStatus(id, st) {
	if (bConnected) {
		qers = new Array();
		qers.push("UPDATE task SET status=" + st + " WHERE id=" + id);
		dump("UPDATE task SET status=" + st + " WHERE id=" + id + "\n");
		mDB.executeSimpleSQLs(qers);
	}
}

function getTaskObjects(id) {
	var __profs = new Array();
	if (bConnected) {
		mDB.selectQuery("SELECT id,parent_id,object_url,scan_depth,resp_code,data_size,dnld_time,content_type FROM scan_data WHERE task_id="+id);
		for (var i = 0; i < mDB.aTableData.length; i++) {
			__profs.push(new Array(mDB.aTableData[i][0], mDB.aTableData[i][1], mDB.aTableData[i][2], mDB.aTableData[i][3], mDB.aTableData[i][4], mDB.aTableData[i][5], mDB.aTableData[i][6], mDB.aTableData[i][7]));
		}
	}
	return __profs;
}

function getScanObject(tsk_id, obj_id) {
	var __profs = new Array();
	if (bConnected) {
		mDB.selectQuery("SELECT id,parent_id,object_url,scan_depth,resp_code,data_size,dnld_time,content_type FROM scan_data WHERE task_id="+tsk_id+" AND id="+obj_id);
		if (mDB.aTableData.length > 0) {
			__profs = new Array(mDB.aTableData[0][0], mDB.aTableData[0][1], mDB.aTableData[0][2], mDB.aTableData[0][3], mDB.aTableData[0][4], mDB.aTableData[0][5], mDB.aTableData[0][6], mDB.aTableData[0][7]);
			if (__profs[1] > 0) {
				mDB.selectQuery("SELECT * FROM scan_data WHERE task_id="+tsk_id+" AND id="+__profs[1]);
				if (mDB.aTableData.length > 0) {
					__profs[9] = mDB.aTableData[0][3];
				}
				else {
					__profs[9] = "undefined";
				}
			}
		}
	}
	return __profs;
}

function getVulner(tsk_id, vuln_id) {
	var __profs = new Array();
	if (bConnected) {
		mDB.selectQuery("SELECT id,object_id,plugin_id,severity,vulner_id,params FROM vulners WHERE task_id="+tsk_id+" AND id="+vuln_id);
		if (mDB.aTableData.length > 0) {
			__profs = new Array(mDB.aTableData[0][0], mDB.aTableData[0][1], mDB.aTableData[0][2], mDB.aTableData[0][3], mDB.aTableData[0][4], mDB.aTableData[0][5]);
			if (__profs[0] > 0) {
				req = "SELECT title FROM vulner_desc WHERE plugin_id='"+__profs[2]+"' AND id="+__profs[4]+ " AND locale='en'";
				mDB.selectQuery(req);
				if (mDB.aTableData.length > 0) {
					__profs[6] = mDB.aTableData[0][0];
				}
				else {
					__profs[6] = "undefined vulnerability";
				}
			}
		}
	}
	return __profs;
}

function getTaskVulners(id) {
	var __profs = new Array();
	if (bConnected) {
		mDB.selectQuery("SELECT id,object_id,plugin_id,severity,vulner_id,params FROM vulners WHERE task_id="+id);
		for (var i = 0; i < mDB.aTableData.length; i++) {
			__profs.push(new Array(mDB.aTableData[i][0], mDB.aTableData[i][1], mDB.aTableData[i][2], mDB.aTableData[i][3], mDB.aTableData[i][4], mDB.aTableData[i][5]));
		}
		for (var i in __profs) {
			req = "SELECT title FROM vulner_desc WHERE plugin_id='"+__profs[i][2]+"' AND id="+__profs[i][4]+ " AND locale='en'";
			mDB.selectQuery(req);
			if (mDB.aTableData.length > 0) {
				__profs[i][6] = mDB.aTableData[0][0];
			}
			else {
				__profs[i][6] = "undefined vulnerability";
			}
		}
	}
	return __profs;
}

function fixTaskInfo(__tsk) {
	if (__tsk[1] == null) {
		__tsk[1] = "&lt;undefined&gt;";
	}
	else {
		mDB.selectQuery("SELECT value FROM profile WHERE name='profile_name' AND profile_id="+__tsk[1]);
		if (mDB.aTableData.length > 0) {
			__tsk[1] = mDB.aTableData[0][0];
		}
	}
	if (__tsk[8] == null) {
		mDB.selectQuery("SELECT object_url FROM scan_data WHERE task_id="+__tsk[0]+" ORDER BY id ASC LIMIT 1");
		if (mDB.aTableData.length > 0) {
			str = mDB.aTableData[0][0];
			str = str.replace(/[^:\/]+:\/\/([^\/]+)\/.*/, "$1");
			__tsk[8] = str;
		}
		else {
			__tsk[8] = "&lt;undefined&gt;";
		}
	}
	return __tsk;
}

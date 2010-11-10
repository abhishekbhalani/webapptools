// JScript source code
// test code for debug DOM structure
// all this code is a subject to review and implementation in native code
var _v8_result = "";

var ActiveXObject = false;

window.XMLHttpRequest = function XMLHttpRequest() {
    this.onreadystatechange = undefined;
    this.readyState	= 4;
            //0 = uninitialized
            //1 = loading
            //2 = loaded
            //3 = interactive
            //4 = complete
    this.responseText = "";
    this.responseXML = "";
    this.status	= 200; // HTTP responce code
    this.statusText	= "OK"; //Returns the status as a string (e.g. "Not Found" or "OK")
    
    this.abort = function() {
        // do nothing
        return;
    };
    this.getAllResponseHeaders = function() {
        return "";
    };
    this.getResponseHeader = function(headername) {
        return "";
    };
    this.open = function(method,URL,async,uname,pswd) {
        msg = "Open XMLHttpRequest\r\n\tmethod: " + method + "\r\n\thref: " + URL + 
                "\r\n\tasync: " + async + "\r\n\tuname: " + uname + "\r\n\tpswd: " + pswd;
		echo(msg);
    };
    this.send = function(content) {
        msg = "XMLHttpRequest sends data";
		echo(msg);
    }
    this.setRequestHeader = function(label,value) {
        msg = "XMLHttpRequest sets header\r\n\t" + label + ": " + value + "\r\n";
		echo(msg);
    }
};

function RunJqueryEvents(obj) {
	var evt_list = $(obj).data("events");
	if (evt_list != null) {
		echo2(obj);
		for (var i in evt_list) {
			for (var j in evt_list[i]) {
				if (evt_list[i][j].handler) {
					echo2("\tFound handler "+i+"["+ j+"]");
					if (i == "click") {
						echo2("\t!! Run !!!");
						evt_list[i][j].handler();
					}
				}
			}
		}
	}
	else {
		//echo2(obj + " hasn't jQuery bindings");
	}
}

function dumpDomTree(obj, ident, num) {
	if (!ident) {
		ident = "";
	}
	if (!num) {
		num = "";
	}
	else {
		num = num + ": ";
	}
	echo2(ident+num+obj);
	ident += "\t";
	for (var i in obj.childNodes) {
		dumpDomTree(obj.childNodes[i], ident, i);
	}
}

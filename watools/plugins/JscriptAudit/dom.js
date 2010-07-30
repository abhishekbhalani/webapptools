// JScript source code
// test code for debug DOM structure
// all this code is a subject to review and implementation in native code
var _v8_result = "";

document.location = location;

var ActiveXObject = false;

function XMLHttpRequest() {
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
        echo("Open XMLHttpRequest\r\n\tmethod: " + method + "\r\n\thref: " + URL + 
                "\r\n\tasync: " + async + "\r\n\tuname: " + uname + "\r\n\tpswd: " + pswd);
    };
    this.send = function(content) {
        echo("XMLHttpRequest sends data");
    }
    this.setRequestHeader = function(label,value) {
        echo("XMLHttpRequest sets header\r\n\t" + label + ": " + value + "\r\n");
    }
};

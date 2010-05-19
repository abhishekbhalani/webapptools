// JScript source code
// test code for debug DOM structure
// all this code is a subject to review and implementation in namtive code
var _v8_result = "";

function setTimeout(arg, t) {
    var X=arg();
     _v8_result += dumpObj(X, "setTimeout function", "", 0);
     _v8_result += "\r\n"
};      

document.images = Array();
document.forms = Array();
function createElement(name, tmpl){
    return document.createElement(name, tmpl);
}

var window;
var self;
var top;

function Window (url, nm) {
    // properties
    this.closed = false;
    this.defaultStatus = "";
    this.document = document;
    this.frames = Array();
    this.length = function() { return this.frames.size(); };
    this.location = location;
    this.name = "";
    this.navigator = navigator;
    this.opener = undefined;
    this.parent = this;
    this.top = top;
    
    this.XMLHttpRequest = true;

    // functions
    this.alert = function(arg) {
        alert(arg);
    };
    this.blur = function() {
        // do nothing
        return;
    };
    this.close = function () {
        this.closed = true;
    };
    this.createPopup = function() {
        var w = new Window(this.location);
        w.parent = this;
        w.opener = this;
        return w;
    };
    this.focus = function() {
        // do nothing
        return;
    };
    this.open = function(url, nm, specs , replace) {
        print("Open new Window\r\n\thref: " + url);
        var w = new Window(url, nm);
        w.parent = this;
        w.opener = this;
        return w;
    };
    this.prompt = function(msg,defaultText) {
        // do nothing
        if (defaultText === undefined) {
            defaultText = "";
        }
        return defaultText;
    };
    this.setTimeout = function(arg, t) {
        setTimeout(arg, t);
    };
    
    // initialize
    if (url === undefined) {
        print("Create new Window without URL");
    }
    else {
        this.location = url;
        print("Create new Window\r\n\thref: " + url);
    };
}

window = new Window();
self = window;
top = window;

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
        print("Open XMLHttpRequest\r\n\tmethod: " + method + "\r\n\thref: " + URL + 
                "\r\n\tasync: " + async + "\r\n\tuname: " + uname + "\r\n\tpswd: " + pswd);
    };
    this.send = function(content) {
        print("XMLHttpRequest sends data");
    }
    this.setRequestHeader = function(label,value) {
        print("XMLHttpRequest sets header\r\n\t" + label + ": " + value + "\r\n");
    }
};


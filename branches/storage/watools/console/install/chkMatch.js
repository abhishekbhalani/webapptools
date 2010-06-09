function chkMatch(txt1, txt2, msg1, msg2, place)
{
    var el1 = document.getElementById(txt1);
    var el2 = document.getElementById(txt2);
    var el3 = document.getElementById(place);
    if(el1 != undefined && el2 != undefined) {
        var st1 = el1.value;
        var st2 = el2.value;
        if (st1 != "" && st2 != "") {
            el3.style.display="inline";
            if (st1 == st2) {
                //alert(msg1);
                el3.innerHTML = msg1;
                return true;
            }
            else {
                //alert(msg2);
                el3.innerHTML = msg2;
                return true;
            }
        }
        else {
            //alert('All blank!');
            el3.style.display = 'none';
        }
    }
    else {
        //alert('Some shit!');
    }
}
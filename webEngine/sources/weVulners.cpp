/*
    scanServer is the HTML processing library
    Copyright (C) 2009 Andrew Abramov stinger911@gmail.com

    This file is part of scanServer

    scanServeris free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    scanServeris distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with scanServer.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <weVulners.h>
#include <weTagScanner.h>

namespace webEngine {

RecordSet* VulnerDesc::ToRS( const string& parentID = "" )
{
    RecordSet* retval = new RecordSet;
    Record* rec = new Record;

    rec->Option(weoID, id);
    rec->Option(weoName, title);
    rec->Option("severity", severity);
    rec->Option("short_desc", shortDesc);
    rec->Option("description", longDesc);
    retval->push_back(*rec);
    return retval;
}

void VulnerDesc::FromRS( RecordSet *rs )
{

}

string VulnerDesc::ToXML()
{
    string retval;

    retval += "<vdesc id='";
    retval += ScreenXML(id);
    retval += "'>\n";
    retval += "<title>";
    retval += ScreenXML(title);
    retval += "</title>\n";
    retval += "<severity>";
    retval += boost::lexical_cast<string>(severity);
    retval += "</severity>\n";
    retval += "<short_desc>";
    retval += ScreenXML(shortDesc);
    retval += "</short_desc>\n";
    retval += "<description>";
    retval += ScreenXML(longDesc);
    retval += "</description>\n";
    retval += "</vdesc>\n";

    return retval;
}

bool VulnerDesc::FromXML(const string& xml)
{
    return false;
}

}

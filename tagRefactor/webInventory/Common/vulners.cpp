/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov stinger911@gmail.com

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <weiBase.h>
#include <weLogger.h>
#include <weProfile.h>
#include <weDispatch.h>
#include "vulners.h"

using namespace webEngine;

extern Dispatch* globalDispatcher;
//extern string cfgFile;
//extern string workDir;

string get_vdesc_list(const string& filter)
{
    string retval = "";
    string data;
    Record filter;
    Record reps;
    wOption opt;
    int i, vulns;
    RecordSet report;

    LOG4CXX_TRACE(iLogger::GetLogger(), "get_vdesc_list");
    filter.objectID = weObjTypeTask;
    filter.Clear();
    reps.objectID = weObjTypeTask;
    reps.Clear();
    reps.Option(weoID);
    reps.Option(weoName);
    retval = "<vdescs>\n";
    vulns = globalDispatcher->Storage()->Get(filter, reps, report);
    for (i = 0; i < vulns; i++)
    {
        opt = report[i].Option(weoID);
        SAFE_GET_OPTION_VAL(opt, data, "");
        if (data.find(filter)) {
            retval += "<vdesc id='";
            retval += ScreenXML(data);
            retval += "'>";
            opt = report[i].Option(weoName);
            SAFE_GET_OPTION_VAL(opt, data, "");
            retval += ScreenXML(data);
            retval += "</vdesc>";
        }
    }
    retval += "</vdescs>";

    return retval;
}

string set_vdesc(const string& id, const VulnerDesc& desc)
{
}

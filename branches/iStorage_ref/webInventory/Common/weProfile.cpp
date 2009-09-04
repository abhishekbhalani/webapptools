/*
    scanServer is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of scanServer

    scanServer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    scanServer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with inventoryScanner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <weHelper.h>
#include <weiStorage.h>
#include "weProfile.h"

RecordSet* WeProfile::ToRS( const string& parentID/* = ""*/ )
{
    RecordSet* res = new RecordSet;
    RecordSet* rsOpts;
    Record* rec;
    string sdata;
    string prfid;
    wOption opt;
    wOptions::iterator it;

    LOG4CXX_TRACE(iLogger::GetLogger(), "WeProfile::ToRS");

    rec = new Record;
    rec->objectID = weObjTypeProfile;

    opt = Option(weoID);
    SAFE_GET_OPTION_VAL(opt, prfid, "0");
    rec->Option(weoID, prfid);

    opt = Option(weoName);
    SAFE_GET_OPTION_VAL(opt, sdata, "");
    rec->Option(weoName, sdata);

    rec->Option(weoParentID, parentID);

    res->push_back(*rec);

    for (it = options.begin(); it != options.end(); it++) {
        sdata = it->first;
        // skip predefined options
        if (sdata == weoID) {
            continue;
        }
        if (sdata == weoName) {
            continue;
        }

        sdata = it->first;
        opt = *(it->second);
        rec = new Record;
        rec->objectID = weObjTypeSysOption;
        rec->Option(weoName, sdata);
        rec->Option(weoParentID, prfid);
        rec->Option(weoTypeID, opt.Which());
        rec->Option(weoValue, opt.Value());
        res->push_back(*rec);
    }
    return res;
}

void WeProfile::FromRS( RecordSet *rs )
{
    Record rec;
    size_t r;
    wOptionVal optVal;
    wOption opt;
    string strData;

    LOG4CXX_TRACE(iLogger::GetLogger(), "WeProfile::FromRS");

    for (r = 0; r < rs->size(); r++)
    {
        rec = (*rs)[r];
        if (rec.objectID == weObjTypeProfile) {
            opt = rec.Option(weoID);
            Option(weoID, opt.Value());

            opt = rec.Option(weoName);
            Option(weoName, opt.Value());
        }
        if (rec.objectID == weObjTypeSysOption) {
            opt = rec.Option(weoName);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            // type doesn't matter here
            // opt = rec.Option(weoTypeID);
            opt = rec.Option(weoValue);
            optVal = opt.Value();
            Option(strData, optVal);
        }
    }
}
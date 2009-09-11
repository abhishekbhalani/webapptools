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
#include <weHelper.h>
#include <weiStorage.h>
#include "weProfile.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash.hpp>

RecordSet* WeProfile::ToRS( const string& parentID/* = ""*/ )
{
    RecordSet* res = new RecordSet;
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

        sdata += prfid;
        sdata += boost::lexical_cast<string>(opt.Which());
        boost::hash<string> strHash;
        size_t hs = strHash(sdata);
        sdata = boost::lexical_cast<string>((unsigned int)hs);
        rec->Option(weoID, sdata);
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
    string strData, sName;
    int tp;
    char c;
    unsigned char uc;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    bool b;
    double d;

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
            // name
            opt = rec.Option(weoName);
            SAFE_GET_OPTION_VAL(opt, sName, "");
            // type 
            opt = rec.Option(weoTypeID);
            SAFE_GET_OPTION_VAL(opt, strData, "8");
            tp = boost::lexical_cast<int>(strData);
            // value
            opt = rec.Option(weoValue);
            strData = boost::lexical_cast<string>(opt.Value());
            switch(tp)
            {
            case 0:
                c = boost::lexical_cast<char>(strData);
                Option(sName, c);
                break;
            case 1:
                uc = boost::lexical_cast<unsigned char>(strData);
                Option(sName, uc);
                break;
            case 2:
                i = boost::lexical_cast<int>(strData);
                Option(sName, i);
                break;
            case 3:
                ui = boost::lexical_cast<unsigned int>(strData);
                Option(sName, ui);
                break;
            case 4:
                l = boost::lexical_cast<long>(strData);
                Option(sName, l);
                break;
            case 5:
                ul = boost::lexical_cast<unsigned long>(strData);
                Option(sName, ul);
                break;
            case 6:
                b = boost::lexical_cast<bool>(strData);
                Option(sName, b);
                break;
            case 7:
                d = boost::lexical_cast<double>(strData);
                Option(sName, d);
                break;
            case 8:
            default:
                Option(sName, strData);
                break;
            }
        }
    }
}
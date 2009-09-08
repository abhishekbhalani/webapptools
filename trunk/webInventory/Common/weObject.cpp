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
#include <weLogger.h>
#include <weHelper.h>
#include <weiStorage.h>
#include "weObject.h"

RecordSet* ScanObject::ToRS( void )
{
    RecordSet* res = new RecordSet;
    RecordSet* rsData;
    Record* rec = new Record;

    LOG4CXX_TRACE(iLogger::GetLogger(), "ScanObject::ToRS");

    rec->objectID = weObjTypeObject;
    rec->Option(weoID, ObjectId);
    rec->Option(weoName, ObjName);
    rec->Option("host", Address);
    res->push_back(*rec);

    return res;
}

void ScanObject::FromRS( RecordSet* rs )
{
    Record rec;
    size_t r;
    wOptionVal optVal;
    wOption opt;
    string strData;

    LOG4CXX_TRACE(iLogger::GetLogger(), "ScanObject::FromRS");

    if (rs->size() > 0)
    {
        rec = (*rs)[0];

        opt = rec.Option(weoID);
        SAFE_GET_OPTION_VAL(opt, strData, "");
        ObjectId = strData;

        opt = rec.Option(weoName);
        SAFE_GET_OPTION_VAL(opt, strData, "");
        ObjName = strData;

        opt = rec.Option("host");
        SAFE_GET_OPTION_VAL(opt, strData, "");
        Address = strData;
    }
}

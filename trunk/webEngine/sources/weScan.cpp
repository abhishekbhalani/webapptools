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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "weScan.h"
#include <weiStorage.h>

namespace webEngine {

ScanInfo::ScanInfo()
{
    startTime = posix_time::second_clock::local_time();
    finishTime = posix_time::not_a_date_time;
    pingTime = posix_time::not_a_date_time;
    status = weScanIdle;
    scanID = "";
}

ScanInfo::~ScanInfo()
{
    // nothing special at this moment
}

RecordSet* ScanInfo::ToRS( const string& parentID/* = ""*/ )
{
    RecordSet* res = new RecordSet;
    RecordSet* rsData;
    Record* rec = new Record;

    rec->objectID = weObjTypeScan;
    rec->Option(weoID, scanID);
    rec->Option(weoParentID, objectID);
    rec->Option(weoProfileID, profileID);
    rec->Option("starttime", posix_time::to_simple_string(startTime));
    rec->Option("finishtime", posix_time::to_simple_string(finishTime));
    rec->Option("pingtime", posix_time::to_simple_string(pingTime));
    rec->Option(weoTaskStatus, status);
    rec->Option("scansize", boost::lexical_cast<string>(scan_data.size()));

    res->push_back(*rec);

    for (size_t i = 0; i < scan_data.size(); i++) {
        rsData = scan_data[i]->ToRS(scanID);
        for (size_t j = 0; j < rsData->size(); j++)
        {
            res->push_back((*rsData)[j]);
        }
        delete rsData;
    }
    return res;
}

void ScanInfo::FromRS( RecordSet* rs )
{
    Record rec;
    size_t r;
    wOptionVal optVal;
    wOption opt;
    string strData;

    LOG4CXX_TRACE(iLogger::GetLogger(), "ScanInfo::FromRS");

    for (r = 0; r < rs->size(); r++)
    {
        rec = (*rs)[r];
        if (rec.objectID == weObjTypeScan) {
            opt = rec.Option(weoID);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            scanID = strData;

            opt = rec.Option(weoParentID);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            objectID = strData;

            opt = rec.Option(weoProfileID);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            profileID = strData;

            opt = rec.Option("starttime");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            startTime = posix_time::time_from_string(strData);

            opt = rec.Option("finishtime");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            finishTime = posix_time::time_from_string(strData);

            opt = rec.Option("pingtime");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            pingTime = posix_time::time_from_string(strData);

            opt = rec.Option(weoTaskStatus);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            scanID = boost::lexical_cast<int>(strData);

//             opt = rec.Option("scansize");
//             SAFE_GET_OPTION_VAL(opt, strData, "");
//             scanID = strData;

            break;
        }
    }
}

ScanData* ScanInfo::GetScanData( const string& baseUrl, const string& realUrl )
{
    ScanData* retval = NULL;

    for (size_t i = 0; i < scan_data.size(); i++) {
        if (scan_data[i]->requestedURL == baseUrl &&
            scan_data[i]->realURL == realUrl)
        {
            LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTask::GetScanData - found existing ScanData");
            retval = scan_data[i];
            break;
        }
    }
    if (retval == NULL)
    {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTask::GetScanData new ScanData");
        retval = new ScanData;
        retval->respCode = 0;
        retval->downloadTime = -1;
        retval->dataSize = -1;
        retval->requestedURL = baseUrl;
        retval->realURL = realUrl;
    }
    return retval;
}

void ScanInfo::SetScanData( ScanData* scData )
{
    size_t i;

    for (i = 0; i < scan_data.size(); i++) {
        if (scan_data[i] == scData)
        {
            LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTask::SetScanData - found existing ScanData");
            break;
        }
    }
    if (i == scan_data.size()) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTask::SetScanData add ScanData to list");
        scan_data.push_back(scData);
    }
}

RecordSet* ScanData::ToRS( const string& parentID/* = ""*/ )
{
    RecordSet* res = new RecordSet;
    Record* rec = new Record;

    rec->objectID = weObjTypeScanData;
    rec->Option(weoID, dataID);
    rec->Option(weoParentID, scanID);
    rec->Option("updata", parentID);
    rec->Option("req_url", requestedURL);
    rec->Option("final_url", realURL);
    rec->Option("response", boost::lexical_cast<string>(respCode));
    rec->Option("data_size", boost::lexical_cast<string>(dataSize));
    rec->Option("download_time", boost::lexical_cast<string>(downloadTime));

    res->push_back(*rec);

    return res;
}

void ScanData::FromRS( RecordSet* rs )
{
    Record rec;
    size_t r;
    wOptionVal optVal;
    wOption opt;
    string strData;

    LOG4CXX_TRACE(iLogger::GetLogger(), "ScanInfo::FromRS");

    for (r = 0; r < rs->size(); r++)
    {
        rec = (*rs)[r];
        if (rec.objectID == weObjTypeTask) {
            opt = rec.Option(weoID);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            dataID = strData;

            opt = rec.Option(weoParentID);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            scanID = strData;

            opt = rec.Option("updata");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            parentID = strData;

            opt = rec.Option("req_url");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            requestedURL = strData;

            opt = rec.Option("final_url");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            realURL = strData;

            opt = rec.Option("response");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            respCode = boost::lexical_cast<int>(strData);

            opt = rec.Option("data_size");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            dataSize = boost::lexical_cast<int>(strData);

            opt = rec.Option("download_time");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            downloadTime = boost::lexical_cast<int>(strData);

            break;
        }
    }
}

} // namespace webEngine

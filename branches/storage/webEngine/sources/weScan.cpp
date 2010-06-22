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
#include <webEngine.h>

#include <weLogger.h>
#include <weHelper.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "weScan.h"
#include <weiStorage.h>

using namespace boost;

namespace webEngine {

ScanInfo::ScanInfo()
{
    startTime = btm::second_clock::local_time();
    finishTime = btm::not_a_date_time;
    pingTime = btm::not_a_date_time;
    status = weScanIdle;
    scanID = "";
}

ScanInfo::~ScanInfo()
{
    //unordered_map<string, weak_ptr<ScanData>>::iterator mit = 
    // nothing special at this moment
//     for (size_t i = 0; i < scan_data.size(); i++) {
//         delete scan_data[i];
//     }
    scan_data.clear();
}

db_recordset* ScanInfo::ToRS( const string& parentID/* = ""*/ )
{
    db_recordset* res = new db_recordset;
    //db_recordset* rsData;
    db_record* rec = new db_record;

/*    rec->objectID = weObjTypeScan;
    rec->Option(weoID, scanID);
    rec->Option(weoParentID, objectID);
    rec->Option(weoProfileID, profileID);
    rec->Option("starttime", btm::to_simple_string(startTime));
    rec->Option("finishtime", btm::to_simple_string(finishTime));
    rec->Option("pingtime", btm::to_simple_string(pingTime));
    rec->Option(weoTaskStatus, status);
    rec->Option("scansize", boost::lexical_cast<string>(scan_data.size()));

    res->push_back(*rec);

/*    for (size_t i = 0; i < scan_data.size(); i++) {
        rsData = scan_data[i]->ToRS(scanID);
        for (size_t j = 0; j < rsData->size(); j++)
        {
            res->push_back((*rsData)[j]);
        }
        delete rsData;
    }*/
    return res;
}

void ScanInfo::FromRS( db_recordset* rs )
{
/*    db_record rec;
    size_t r;
    we_variant optVal;
    we_option opt;
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
            try {
                startTime = btm::time_from_string(strData);
            } catch (std::exception&) {
                startTime = btm::not_a_date_time;
            }

            opt = rec.Option("finishtime");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            try {
                finishTime = btm::time_from_string(strData);
            } catch (std::exception&) {
                finishTime = btm::not_a_date_time;
            }

            opt = rec.Option("pingtime");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            try {
                pingTime = btm::time_from_string(strData);
            } catch (std::exception&) {
                pingTime = btm::not_a_date_time;
            }

            opt = rec.Option(weoTaskStatus);
            try {
                status = (weScanStatus) boost::lexical_cast<int>(opt.Value());
            }
            catch(std::exception&) {
                status = weScanIdle;
            }

//             opt = rec.Option("scansize");
//             SAFE_GET_OPTION_VAL(opt, strData, "");
//             scan_id = strData;

            break;
        }
    }*/
}

shared_ptr<ScanData> ScanInfo::GetScanData( const string& object_url )
{
    shared_ptr<ScanData> retval;
    ScanInfo::iterator mit = scan_data.find(object_url);

    if (mit != scan_data.end()) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTask::GetScanData - found existing ScanData");
        retval = mit->second;
    }
    else {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTask::GetScanData new ScanData");
        retval.reset(new ScanData);
        retval->resp_code = 0;
        retval->download_time = -1;
        retval->data_size = -1;
        retval->object_url = object_url;
        retval->parsed_data.reset();        
        scan_data[object_url] = retval;
    }
    return retval;
}

void ScanInfo::SetScanData( const string& object_url, scan_data_ptr scData )
{
    scan_data[object_url] = scData;
}

ScanData::ScanData()
{
    parsed_data.reset();
    scan_depth = 0;
}

ScanData::~ScanData()
{
    parsed_data.reset();
}

db_recordset* ScanData::ToRS( const string& parentID/* = ""*/ )
{
    db_recordset* res = new db_recordset;
    db_record* rec = new db_record;

/*    rec->objectID = weObjTypeScanData;
    rec->Option(weoID, data_id);
    rec->Option(weoParentID, scan_id);
    rec->Option("updata", parentID);
    rec->Option("req_url", object_url);
    rec->Option("response", boost::lexical_cast<string>(resp_code));
    rec->Option("data_size", boost::lexical_cast<string>(data_size));
    rec->Option("download_time", boost::lexical_cast<string>(download_time));
    rec->Option("scanning_depth", boost::lexical_cast<string>(scan_depth));

    res->push_back(*rec);*/

    return res;
}

void ScanData::FromRS( db_recordset* rs )
{
/*    db_record rec;
    size_t r;
    we_variant optVal;
    we_option opt;
    string strData;

    LOG4CXX_TRACE(iLogger::GetLogger(), "ScanInfo::FromRS");

    for (r = 0; r < rs->size(); r++)
    {
        rec = (*rs)[r];
        if (rec.objectID == weObjTypeTask) {
            opt = rec.Option(weoID);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            data_id = strData;

            opt = rec.Option(weoParentID);
            SAFE_GET_OPTION_VAL(opt, strData, "");
            scan_id = strData;

            opt = rec.Option("updata");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            parent_id = strData;

            opt = rec.Option("req_url");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            object_url = strData;

//             opt = rec.Option("final_url");
//             SAFE_GET_OPTION_VAL(opt, strData, "");
//             realURL = strData;

            opt = rec.Option("response");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            resp_code = boost::lexical_cast<int>(strData);

            opt = rec.Option("data_size");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            data_size = boost::lexical_cast<int>(strData);

            opt = rec.Option("download_time");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            download_time = boost::lexical_cast<int>(strData);

            opt = rec.Option("scanning_depth");
            SAFE_GET_OPTION_VAL(opt, strData, "");
            scan_depth = boost::lexical_cast<int>(strData);

            break;
        }
    }*/
}

} // namespace webEngine

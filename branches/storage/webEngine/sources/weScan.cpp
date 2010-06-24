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
#include <weScan.h>
// for table names
#include <weiStorage.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

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
        rsData = scan_data[i]->to_dataset(scanID);
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

    LOG4CXX_TRACE(iLogger::GetLogger(), "ScanInfo::from_dataset");

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
    response.reset();
    parsed_data.reset();
    scan_depth = 0;
}

ScanData::~ScanData()
{
    response.reset();
    parsed_data.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool ScanData::to_dataset( db_recordset& dataset )
///
/// @brief  Converts a ScanData to a dataset. 
///
/// @author A. Abramov
/// @date   24.06.2010
///
/// @param [in,out] dataset The dataset. 
///
/// @return true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanData::to_dataset( db_cursor& dataset )
{
    bool retval = true;

    try {
        dataset[weObjTypeScanData "." weoID] = data_id;
        dataset[weObjTypeScanData "." "task_id"] = scan_id;
        dataset[weObjTypeScanData "." weoParentID] = parent_id;
        dataset[weObjTypeScanData "." "object_url"] = object_url;
        dataset[weObjTypeScanData "." "scan_depth"] = scan_depth;
        dataset[weObjTypeScanData "." "resp_code"] = resp_code;
        dataset[weObjTypeScanData "." "data_size"] = data_size;
        dataset[weObjTypeScanData "." "dnld_time"] = download_time;
        dataset[weObjTypeScanData "." "content_type"] = content_type;
    }
    catch (std::exception &e) {
        // may be out_of_range or bad_cast
        LOG4CXX_ERROR(iLogger::GetLogger(), "ScanData::to_dataset exception: " << e.what());
        retval = false;
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool ScanData::from_dataset( const db_cursor& dataset )
///
/// @brief  Initializes this object from the given from dataset. 
///
/// @author A. Abramov
/// @date   24.06.2010
///
/// @param  dataset The dataset. 
///
/// @return true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool ScanData::from_dataset( const db_cursor& dataset )
{
    bool retval = true;

    try {
        parsed_data.reset();
        response.reset();

        data_id = dataset[weObjTypeScanData "." weoID].get<string>();
        scan_id = dataset[weObjTypeScanData "." "task_id"].get<string>();
        parent_id = dataset[weObjTypeScanData "." weoParentID].get<string>();
        object_url = dataset[weObjTypeScanData "." "object_url"].get<string>();
        scan_depth = dataset[weObjTypeScanData "." "scan_depth"].get<int>();
        resp_code = dataset[weObjTypeScanData "." "resp_code"].get<int>();
        data_size = dataset[weObjTypeScanData "." "data_size"].get<int>();
        download_time = dataset[weObjTypeScanData "." "dnld_time"].get<int>();
        content_type = dataset[weObjTypeScanData "." "content_type"].get<string>();
    }
    catch (std::exception &e) {
        // may be out_of_range or bad_cast
        LOG4CXX_ERROR(iLogger::GetLogger(), "ScanData::from_dataset exception: " << e.what());
        retval = false;
    }
    return retval;
}

} // namespace webEngine

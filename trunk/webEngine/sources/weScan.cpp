/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
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

shared_ptr<ScanData> ScanInfo::GetScanData( const string& object_url )
{
    shared_ptr<ScanData> retval;
    ScanInfo::iterator mit = scan_data.find(object_url);

    if (mit != scan_data.end()) {
        LOG4CXX_DEBUG(iLogger::GetLogger(), "WeTask::GetScanData - found existing ScanData");
        retval = mit->second;
    } else {
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
    data_id.clear();
    parent_id.clear();
    scan_id.clear();
    object_url.clear();
    resp_code = 0;
    data_size = 0;
    download_time = 0;
    scan_depth = 0;
    content_type.clear();
    response.reset();
    parsed_data.reset();
}

ScanData::~ScanData()
{
    response.reset();
    parsed_data.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn bool ScanData::to_dataset( db_cursor& dataset )
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
bool ScanData::to_dataset( db_record& dataset )
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
    } catch (std::exception &e) {
        // may be out_of_range or bad_cast
		LOG4CXX_ERROR(iLogger::GetLogger(), "ScanData::to_dataset exception: " << std::string(e.what()));
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
bool ScanData::from_dataset( const db_record& dataset )
{
    bool retval = true;
    int id = 0;

    try {
        parsed_data.reset();
        response.reset();

        data_id = dataset[weObjTypeScanData "." weoID].get<string>();
        id++;
        scan_id = dataset[weObjTypeScanData "." "task_id"].get<string>();
        id++;
        parent_id = dataset[weObjTypeScanData "." weoParentID].get<string>();
        id++;
        object_url = dataset[weObjTypeScanData "." "object_url"].get<string>();
        id++;
        scan_depth = dataset[weObjTypeScanData "." "scan_depth"].get<int>();
        id++;
        resp_code = dataset[weObjTypeScanData "." "resp_code"].get<int>();
        id++;
        data_size = dataset[weObjTypeScanData "." "data_size"].get<int>();
        id++;
        download_time = dataset[weObjTypeScanData "." "dnld_time"].get<int>();
        id++;
        content_type = dataset[weObjTypeScanData "." "content_type"].get<string>();
    } catch (std::exception &e) {
        // may be out_of_range or bad_cast
        LOG4CXX_ERROR(iLogger::GetLogger(), "ScanData::from_dataset exception at " << id <<L": " << std::string(e.what()));
        retval = false;
    }
    return retval;
}

} // namespace webEngine

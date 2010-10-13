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
#ifndef __WESCAN_H__
#define __WESCAN_H__
#include <weTagScanner.h>
#include <weBlob.h>
#include <weiParser.h>
#include <weiTransport.h>
#include <weDbstruct.h>
#include <boost/unordered_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>

namespace btm = boost::posix_time;

namespace webEngine {

class db_recordset;
class html_document;

class ScanData {
public:
    ScanData();
    ~ScanData();

    string  data_id;
    string  parent_id;
    string  scan_id;
    string  object_url;
    int     resp_code;
    int     data_size;
    int     download_time;
    int     scan_depth;
    string  content_type;
    i_document_ptr parsed_data;
    i_response_ptr response;

    bool to_dataset( db_record& dataset );
    bool from_dataset( const db_record& dataset );
};

typedef boost::shared_ptr<ScanData> scan_data_ptr;

class ScanInfo {
    typedef std::map<string, scan_data_ptr >    scan_map;
public:
    typedef scan_map::iterator  iterator;

    enum weScanStatus {
        ScanStatusMask = 0xFFFFFFFF
    };
    ScanInfo();
    ~ScanInfo();

    static const weScanStatus weScanIdle     = (weScanStatus)0x0000;
    static const weScanStatus weScanRunning  = (weScanStatus)0x0001;
    static const weScanStatus weScanPaused   = (weScanStatus)0x0002;
    static const weScanStatus weScanStopped  = (weScanStatus)0x0003;
    static const weScanStatus weScanFinished = (weScanStatus)0x0004;
    static const weScanStatus weScanError    = (weScanStatus)0x0005;

    string       scanID;
    string       objectID;
    string       profileID;
    btm::ptime   startTime;
    btm::ptime   finishTime;
    btm::ptime   pingTime;
    weScanStatus status;

    //void push_back(ScanData* elem) { scan_data.push_back(elem);}

    db_recordset* ToRS( const string& parentID = "" );
    void FromRS( db_recordset *rs );

    boost::shared_ptr<ScanData> GetScanData( const string& object_url );
    void SetScanData(const string& object_url, scan_data_ptr scData);

protected:
    //boost::unordered_map<string, scan_data_ptr > scan_data;
    scan_map scan_data;
};

} // namespace webEngine

#endif //__WESCAN_H__

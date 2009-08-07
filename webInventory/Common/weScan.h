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
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost;

class WeScanData;

class WeScan
{
public:
    enum weScanStatus {
        ScanStatusMask = 0xFFFFFFFF
    };
    WeScan();
    ~WeScan();

    static const weScanStatus weScanIdle     = (weScanStatus)0x0000;
    static const weScanStatus weScanRunning  = (weScanStatus)0x0001;
    static const weScanStatus weScanPaused   = (weScanStatus)0x0002;
    static const weScanStatus weScanStopped  = (weScanStatus)0x0003;
    static const weScanStatus weScanFinished = (weScanStatus)0x0004;
    static const weScanStatus weScanError    = (weScanStatus)0x0005;

    string            scanID;
    string            objectID;
    posix_time::ptime startTime;
    posix_time::ptime finishTime;
    posix_time::ptime pingTime;
    weScanStatus      status;  

    void push_back(WeScanData* elem) { scan_data.push_back(elem);}

    string ToXml( void );
    void FromXml( string input );
    void FromXml( WeTagScanner& sc, int token = -1 );

protected:
    vector<WeScanData*> scan_data;
};

class WeScanData
{
public:
    string  dataID;
    string  parentID;
    string  scanID;
    string  requestedURL;
    string  realURL;  
    int     respCode;
    int     dataSize;
    int     downloadTime;

    string ToXml( void );
    void FromXml( string input );
    void FromXml( WeTagScanner& sc, int token = -1 );
};
#endif //__WESCAN_H__

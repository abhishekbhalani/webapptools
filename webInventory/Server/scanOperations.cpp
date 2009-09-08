/*
    inventoryScanner is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of inventoryScanner

    inventoryScanner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    inventoryScanneris distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with inventoryScanner.  If not, see <http://www.gnu.org/licenses/>.
*//************************************************************
 * @file   scanOperations.cpp
 *
 * @brief  Operations to process scan-related queries
 *
 * This is the sample implementation and the subject to change
 * @author	Aabramov
 * @date	05.08.2009
 **************************************************************/

#include "messages.h"
#include <fstream>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/operations.hpp>
#ifdef WIN32
#include <process.h>
#else
#   include <spawn.h>
#endif //WIN32
#include <weiBase.h>
#include <weLogger.h>
#include <weDispatch.h>
#include <weScan.h>
#include <weTagScanner.h>
#ifdef WIN32
#define NOGDI
#include <windows.h>
#endif

namespace fs = boost::filesystem;
using namespace webEngine;

extern Dispatch* globalDispatcher;

void load_scan_list( vector<ScanInfo*>& scans_list, string criteria = "" )
{
    Record filter;
    Record reps;
    RecordSet report;
    int scans, i;

    LOG4CXX_TRACE(iLogger::GetLogger(), "load_scan_list");
    filter.objectID = weObjTypeScan;
    filter.Clear();
    reps.objectID = weObjTypeScan;
    reps.Clear();
    scans = globalDispatcher->Storage()->Get(filter, reps, report);
    for (i = 0; i < scans; i++)
    {
        RecordSet local;
        ScanInfo* scn;

        scn = new ScanInfo();
        local.push_back(report[i]);
        scn->FromRS(&local);
        scans_list.push_back(scn);
    }
}

ScanInfo* load_scan( const string& id )
{
    Record filter;
    Record reps;
    RecordSet report;
    int scans, i;
    ScanInfo* scn = NULL;

    LOG4CXX_TRACE(iLogger::GetLogger(), "load_scan_list");
    filter.objectID = weObjTypeScan;
    filter.Clear();
    filter.Option(weoID, id);
    reps.objectID = weObjTypeScan;
    reps.Clear();
    scans = globalDispatcher->Storage()->Get(filter, reps, report);
    if (scans > 0)
    {
        scn->FromRS(&report);
        filter.objectID = weObjTypeScanData;
        filter.Clear();
        filter.Option(weoParentID, id);
        reps.objectID = weObjTypeScanData;
        reps.Clear();
        report.clear();
        scans = globalDispatcher->Storage()->Get(filter, reps, report);
        for (i = 0; i < scans; i++)
        {
            RecordSet local;
            ScanData *dat = new ScanData;
            
            local.push_back(report[i]);
            dat->FromRS(&local);
            scn->push_back(dat);
        }
    }

    return scn;
}

ScanList* get_scan_list(const string& criteria/* = ""*/)
{
    vector<ScanInfo*>  scan_list;
    ScanList *lst = NULL;
    ScanInf *tsk = NULL;
    wOption    opt;
    string sdata;
    size_t t;

    load_scan_list(scan_list);
    lst = new ScanList;
    for(t = 0; t < scan_list.size(); t++)
    {
        tsk = new ScanInf;

        tsk->ScanId = scan_list[t]->scanID;
        tsk->ObjectId = scan_list[t]->objectID;
        tsk->StartTime = posix_time::to_simple_string(scan_list[t]->startTime);
        tsk->FinishTime = posix_time::to_simple_string(scan_list[t]->finishTime);
        tsk->status = scan_list[t]->status;
        lst->push_back(*tsk);
    }
    scan_list.clear();
    return lst;
}

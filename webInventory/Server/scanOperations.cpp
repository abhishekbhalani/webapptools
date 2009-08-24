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
    string report = "";
    int scans = 0;

    scans = globalDispatcher->Storage()->ScanReport("<report><scan value='*' /></report>", report);
    if (scans > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        int xml_pos;
        StrStream st(report.c_str());
        TagScanner sc(st);
        ScanInfo* scn;
        string tag;

        while(in_parsing) {
            xml_pos = sc.GetPos();
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(iLogger::GetLogger(), "load_scan_list - parsing error");
                in_parsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(iLogger::GetLogger(), "load_scan_list - parsing EOF");
                in_parsing = false;
                break;
            case wstTagStart:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "report")) {
                        scans_list.clear();
                        parsing_level++;
                        break;
                    }
                }
                if (parsing_level == 1) {
                    if (iequals(tag, "scan")) {
                        scn = new ScanInfo();
                        // go back to the start of the TAG
                        scn->FromXml(sc, t);
                        scans_list.push_back(scn);
                        break;
                    }
                }
                LOG4CXX_WARN(iLogger::GetLogger(), "load_scan_list - unexpected tag: " << tag);
                in_parsing = false;
                break;
            case wstTagEnd:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "report")) {
                        in_parsing = false;
                    }
                }
                break;
            }
        }
    }
}

ScanInfo* load_scan( const string& id )
{
    string req;
    string report = "";
    int tasks = 0;
    ScanInfo* tsk;

    req = "<report><scan value='" + id + "' /></report>";
    tasks = globalDispatcher->Storage()->ScanReport(req, report);
    tsk = NULL;
    if (tasks > 0)
    {   // parse the response
        bool in_parsing = true;
        int parsing_level = 0;
        int xml_pos;
        StrStream st(report.c_str());
        TagScanner sc(st);
        string tag;

        while(in_parsing) {
            xml_pos = sc.GetPos();
            int t = sc.GetToken();
            switch(t)
            {
            case wstError:
                LOG4CXX_WARN(iLogger::GetLogger(), "load_scan - parsing error");
                in_parsing = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(iLogger::GetLogger(), "load_scan - parsing EOF");
                in_parsing = false;
                break;
            case wstTagStart:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "report")) {
                        parsing_level++;
                        break;
                    }
                }
                if (parsing_level == 1) {
                    if (iequals(tag, "scan")) {
                        tsk = new ScanInfo();
                        // go back to the start of the TAG
                        tsk->FromXml(sc, t);
                        // stop parsing - only first task need
                        in_parsing = false; 
                        break;
                    }
                }
                LOG4CXX_WARN(iLogger::GetLogger(), "load_scan - unexpected tag: " << tag);
                in_parsing = false;
                break;
            case wstTagEnd:
                tag = sc.GetTagName();
                if (parsing_level == 0)
                {
                    if (iequals(tag, "report")) {
                        in_parsing = false;
                    }
                }
                break;
            }
        }
    }
    return tsk;
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

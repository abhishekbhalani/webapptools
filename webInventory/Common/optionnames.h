/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __OPTIONSNAMES_H__
#define __OPTIONSNAMES_H__

//////////////////////////////////////////////////////////////////////////
// Define options names
// It's a great idea to keep this file synchronous whith the
// weOptions.h from the webEngine package
//////////////////////////////////////////////////////////////////////////
/// object's human readable name or description (string)
#define weoName              "name"
/// object's identifier (string)
#define weoID                "id"
/// task status (idle, run, etc) (integer)
#define weoTaskStatus        "status"
/// task completion (percents) (integer)
#define weoTaskCompletion    "completion"
#define weoTransport         "TransportName"
#define weoParser            "ParserName"
/// put all founded links into the processing queue (bool)
#define weoFollowLinks       "FollowLinks"
/// automatically load images as WeRefrenceObject (bool)
#define weoLoadImages        "LoadImages"
/// automatically load scripts as WeRefrenceObject (bool)
#define weoLoadScripts       "LoadScripts"
/// automatically load frames as WeRefrenceObject (bool)
#define weoLoadFrames        "LoadFrames"
/// automatically load iframes as WeRefrenceObject (bool)
#define weoLoadIframes       "LoadIframes"
/// collapse multiple spaces into one then HTML parse (bool)
#define weoCollapseSpaces    "CollapseSpaces"
/// do not leave domain of the request (second-level or higher) (bool)
#define weoStayInDomain      "StayInDomain"
/// includes weoStayInDomain (bool)
#define weoStayInHost        "StayInHost"
/// includes woeStayInHost & weoStayInDomain (bool)
#define weoStayInDir         "StayInDir"
/// start response processing automatically (bool)
#define weoAutoProcess       "AutoProcess"
/// controls the relocation loops and duplicates (bool)
#define weoCheckForLoops     "CheckForLoops"
/// base URL for processing (bool)
#define weoBaseURL           "BaseURL"
/// links following depth (integer)
#define weoScanDepth         "ScanDepth"
/// logging level (integer)
#define weoLogLevel          "LogLevel"
/// number of parallel requests to transport (integer)
#define weoParallelReq       "ParallelReq"
/// ignore URL parameters (bool)
#define weoIgnoreUrlParam    "noParamUrl"
//////////////////////////////////////////////////////////////////////////
// Define options typenames
//////////////////////////////////////////////////////////////////////////
#define weoTypeInt           "2"
#define weoTypeUInt          "3"
#define weoTypeBool          "6"
#define weoTypeString        "8"
//////////////////////////////////////////////////////////////////////////
// Define task statuses
//////////////////////////////////////////////////////////////////////////
#define WI_TSK_IDLE     0
#define WI_TSK_RUN      1
#define WI_TSK_PAUSED   2
#define WI_TSK_MAX      3
//////////////////////////////////////////////////////////////////////////

#endif //__OPTIONSNAMES_H__

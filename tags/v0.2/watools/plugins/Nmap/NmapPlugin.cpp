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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include "NmapPlugin.h"
#include <weDispatch.h>
#include <weTask.h>
#include <weScan.h>
#include "version.h"
#include "nmap.xpm"

static string xrc = "<plugin><category label='Basic settings' name='generic'>\
 <option label='Plugin version' type='8' control='string' readonly='1'>%version%</option>\
 <option name='nmapDiscovery/executable' label='Path to Nmap' type='8' control='string' mode='file'>\"C:\\nmap.exe\"</option>\
 <option name='nmapDiscovery/ports' label='Ports to scan' type='8' control='string'>1-1000,10000,20000</option>\
  <category label='Scan techniques' name='tech'>\
   <option name='nmapDiscovery/syn' label='TCP SYN' type='6' control='bool' chkbox='1'>1</option>\
   <option name='nmapDiscovery/con' label='TCP Connect' type='6' control='bool' chkbox='1'>0</option>\
   <option name='nmapDiscovery/ack' label='TCP ACK' type='6' control='bool' chkbox='1'>0</option>\
   <option name='nmapDiscovery/win' label='TCP Window' type='6' control='bool' chkbox='1'>0</option>\
   <option name='nmapDiscovery/mai' label='TCP Maimon' type='6' control='bool' chkbox='1'>0</option>\
   <option name='nmapDiscovery/null' label='TCP Null' type='6' control='bool' chkbox='1'>0</option>\
   <option name='nmapDiscovery/fin' label='TCP FIN' type='6' control='bool' chkbox='1'>0</option>\
   <option name='nmapDiscovery/xmas' label='TCP Xmas' type='6' control='bool' chkbox='1'>0</option>\
  </category>\
 <option name='nmapDiscovery/service' label='Perform service detection' type='6' control='bool' chkbox='1'>0</option>\
 <option name='nmapDiscovery/opsys' label='Perform OS detection' type='6' control='bool' chkbox='1'>0</option>\
 <option name='nmapDiscovery/integration' label='Use results in scanning' type='6' control='bool' chkbox='1'>1</option>\
 <option name='nmapDiscovery/options' label='Other Nmap options' type='8' control='string'></option>\
</category></plugin>";

namespace fs = boost::filesystem;
using namespace webEngine;

NmapDiscovery::NmapDiscovery( Dispatch* krnl, void* handle /*= NULL*/ ) :
    iInventory(krnl, handle)
{
    pluginInfo.IfaceName = "Nmap discovery";
    pluginInfo.IfaceList.push_back("nmapDiscovery");
    pluginInfo.PluginDesc = "Nmap host and ports discovery";
    pluginInfo.PluginId = "F7238FAED048"; // {A9E0E5AF-51C2-4dbf-A562-F7238FAED048}
    pluginInfo.PluginIcon = WeXpmToStringList(nmap_xpm, sizeof(nmap_xpm) / sizeof(char*) );

    /// @todo: change XRC to set the fields values
    string ver = AutoVersion::FULLVERSION_STRING;
    ver += " ";
    ver += AutoVersion::STATUS;
#ifdef WIN32
    ver += " (Windows)";
#else
    ver += " (Linux)";
#endif
#ifdef _DEBUG
    ver += " SVN build ";
    ver += boost::lexical_cast<string>(AutoVersion::BUILDS_COUNT);
#endif
    boost::replace_first(xrc, "%version%", ver);
    priority = 0;
}

NmapDiscovery::~NmapDiscovery(void)
{
}

void* NmapDiscovery::GetInterface( const string& ifName )
{
    if (iequals(ifName, "nmapDiscovery"))
    {
        usageCount++;
        return (void*)(this);
    }
    return iInventory::GetInterface(ifName);
}

const string NmapDiscovery::GetSetupUI( void )
{

    return xrc;
}

void NmapDiscovery::ApplySettings( const string& xmlData )
{

}

void NmapDiscovery::ProcessResponse(iResponse *resp)
{
    // we don't process any incoming objects
}

void NmapDiscovery::Start(Task* tsk)
{
    string cmdline;
    string param;
    char* tmName;

    LOG4CXX_INFO(logger, "NmapDiscovery Start");
    wOption opt = tsk->Option("nmapDiscovery/executable");
    SAFE_GET_OPTION_VAL(opt, cmdline, "");
    if (cmdline == "") {
        LOG4CXX_FATAL(logger, "NmapDiscovery: can't find nmap executable");
        return;
    }
    cmdline += " ";
    opt = tsk->Option("nmapDiscovery/ports");
    SAFE_GET_OPTION_VAL(opt, param, "");
    if (param != "")
    {
        cmdline += "-p " + param + " ";
    }
    if (tsk->IsSet("nmapDiscovery/syn")) {
        cmdline += "-sS ";
    }
    if (tsk->IsSet("nmapDiscovery/con")) {
        cmdline += "-sT ";
    }
    if (tsk->IsSet("nmapDiscovery/ack")) {
        cmdline += "-sA ";
    }
    if (tsk->IsSet("nmapDiscovery/win")) {
        cmdline += "-sW ";
    }
    if (tsk->IsSet("nmapDiscovery/mai")) {
        cmdline += "-sM ";
    }
    if (tsk->IsSet("nmapDiscovery/null")) {
        cmdline += "-sN ";
    }
    if (tsk->IsSet("nmapDiscovery/fin")) {
        cmdline += "-sF ";
    }
    if (tsk->IsSet("nmapDiscovery/xmas")) {
        cmdline += "-sX ";
    }
    if (tsk->IsSet("nmapDiscovery/service")) {
        cmdline += "-sV ";
    }
    if (tsk->IsSet("nmapDiscovery/opsys")) {
        cmdline += "-O ";
    }
    opt = tsk->Option("nmapDiscovery/options");
    SAFE_GET_OPTION_VAL(opt, param, "");
    if (param != "")
    {
        cmdline += param + " ";
    }
    opt = tsk->Option("scan_host");
    SAFE_GET_OPTION_VAL(opt, param, "");
    if (param == "")
    {
        LOG4CXX_FATAL(logger, "NmapDiscovery: can't find host to discovery!");
        return;
    }
    cmdline += param + " ";
    ScanData* scData = tsk->GetScanData(param, param);
    if (scData->dataID == "")
    {
        scData->dataID = kernel->Storage()->GenerateID(weObjTypeScan);
        scData->respCode = 200;
        scData->downloadTime = 0;
        scData->dataSize = 0;
    }


    tmName = tmpnam(NULL);
    if (tmName == NULL) {
        tmName = "nmapDiscovery";
    }
    cmdline += "-oX ";
    cmdline += tmName;
    LOG4CXX_DEBUG(logger, "NmapDiscovery: run " << cmdline );
    time_t tm1 = time(NULL);
    ::system(cmdline.c_str());
    time_t tm2 = time(NULL);
    scData->downloadTime = (int)(tm2 - tm1);
    try{
        size_t fsize = fs::file_size(fs::path(tmName));
        scData->dataSize = fsize;
        ifstream ifs(tmName);
        char* buff = new char[fsize + 10];
        if(buff != NULL)
        {
            memset(buff, 0, fsize+10);
            ifs.read(buff, fsize);
            param = buff;
            delete buff;
        }
    }
    catch ( const std::exception & e )
    {
        LOG4CXX_ERROR(logger, "NmapDiscovery: fileRead error: " << e.what());
    }
    remove(tmName);
    tsk->SetScanData(scData);
    tsk->AddVulner("nmap-0001", param, scData->dataID);
    LOG4CXX_INFO(logger, "NmapDiscovery finished");
}

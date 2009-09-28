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
#include "NmapPlugin.h"
#include "version.h"
#include "nmap.xpm"

static string xrc = "<plugin><category label='Basic settings' name='generic'>\
 <option label='Plugin version' type='8' control='string' readonly='1'>%version%</option>\
 <option name='nmapDiscovery/executable' label='Path to Nmap' type='8' control='string'>\"C:\\nmap.exe\"</option>\
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
 <option name='nmapDiscovery/inegration' label='Use results in scanning' type='6' control='bool' chkbox='1'>1</option>\
 <option name='nmapDiscovery/options' label='Other Nmap options' type='8' control='string'></option>\
</category></plugin>";

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
#ifdef _WIN32_WINNT
    ver += " (Windows)";
#else
    ver += " (Linux)";
#endif
#ifdef _DEBUG
    ver += " SVN build ";
    ver += boost::lexical_cast<string>(AutoVersion::BUILDS_COUNT);
#endif
    boost::replace_first(xrc, "%version%", ver);
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
    LOG4CXX_TRACE(logger, "NmapDiscovery Start");
}

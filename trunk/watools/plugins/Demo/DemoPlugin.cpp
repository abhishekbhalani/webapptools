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
#include "DemoPlugin.h"
#include "version.h"
#include "demoPlugin.xpm"

static string xrc = "<plugin>\
<category label='Basic settings' name='generic'>\
 <option name='demoPlugin/Parameter1' label='String Parameter' type='8' control='string'>text parameter</option>\
 <option name='demoPlugin/Parameter2' label='Bool parameter' type='6' control='bool' checkbox='1'>true</option>\
 <option name='demoPlugin/Parameter3' label='Enum parameter' type='8' control='enum'>Choice1;Choice2;Choice3</option>\
 <option name='' label='Options'>&lt;composed&gt;\
  <option name='demoPlugin/Parameter4' label='Option 1' type='6' control='bool' chkbox='1'>true</option>\
  <option name='demoPlugin/Parameter5' label='Option 2' type='6' control='bool' chkbox='1'>false</option>\
  <option name='demoPlugin/Parameter6' label='Option 3' type='6' control='bool'>1</option>\
  <option name='demoPlugin/Parameter7' label='Option 4' type='6' control='bool'>0</option>\
  <option name='demoPlugin/Parameter8' label='Option 5' type='6' control='bool' chkbox='1'>yes</option>\
  <option name='demoPlugin/Parameter9' label='Option 6' type='6' control='bool' chkbox='1'>no</option>\
 </option>\
</category>\
</plugin>";

DemoPlugin::DemoPlugin( Dispatch* krnl, void* handle /*= NULL*/ ) :
    iPlugin(krnl, handle)
{
    pluginInfo.IfaceName = "demoPlugin";
    pluginInfo.IfaceList.push_back("demoPlugin");
    pluginInfo.PluginDesc = "Demo plugin";
    pluginInfo.PluginId = "2E6587B86F76";
    pluginInfo.PluginIcon = WeXpmToStringList(demoPlugin_xpm, sizeof(demoPlugin_xpm) / sizeof(char*) );
}

DemoPlugin::~DemoPlugin(void)
{
}

void* DemoPlugin::GetInterface( const string& ifName )
{
    if (iequals(ifName, "demoPlugin")) {
        usageCount++;
        return (void*)(this);
    }
    return iPlugin::GetInterface(ifName);
}

const string DemoPlugin::GetSetupUI( void )
{
    /// @todo: change XRC to set the fields values
    return xrc;
}

void DemoPlugin::ApplySettings( const string& xmlData )
{

}
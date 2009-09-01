/*
    webInventory is the web-application audit programm
    Copyright (C) 2009 Andrew Abramov stinger911@gmail.com

    This file is part of webInventory

    webInventory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webInventory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webInventory.  If not, see <http://www.gnu.org/licenses/>.
*/
/***************************************************************
 * @file      wiPlugin.cpp
 * @brief     Defines wiPlugin interface
 * @author    Andrew "Stinger" Abramov (stinger@lab18.net)
 * @date      01.09.2009
 **************************************************************/
 #include "wiPlugin.h"

wiPlugin::wiPlugin(wxDynamicLibrary* lib/* = NULL*/)
{
	m_lib = NULL;
}

wiPlugin::~wiPlugin()
{
	if (m_lib)
	{
		m_lib->Unload();
	}
}

wiPlugin* wiPlugin::Init(wxString pluginName)
{
    wxDynamicLibrary *lib;
    wiPlugin* plg;
    TPfnInit fnInit;

    plg = NULL;
    lib = new wxDynamicLibrary();
    if (lib) {
        if (lib->Load(pluginName)) {
            if (lib->HasSymbol(wxT("PluginFactory"))) {
                fnInit = (TPfnInit)lib->GetSymbol(wxT("PluginFactory"));
                plg = (wiPlugin*)fnInit(lib);
            }
        }
    }
    return plg;
}


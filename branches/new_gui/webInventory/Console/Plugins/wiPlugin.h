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
 * @file      wiPlugin.h
 * @brief     Declares wiPlugin interface
 * @author    Andrew "Stinger" Abramov (stinger@lab18.net)
 * @date      01.09.2009
 **************************************************************/
#ifndef WIPLUGIN_H
#define WIPLUGIN_H

#include <wx/wx.h>
#include <wx/list.h>
#include <wx/string.h>
#include <wx/image.h>
#include <wx/dynlib.h>

#if defined(__WXMSW__)
	#define DLLEXPORT	extern "C" __declspec( dllexport )
#else
	#define DLLEXPORT   extern "C" __attribute__ ((visibility("default")))
#endif

typedef void* (*TPfnInit)(wxDynamicLibrary* lib);

class wiPlugin
{
    public:
        wiPlugin(wxDynamicLibrary* lib = NULL);
        virtual ~wiPlugin();

        virtual wxString* GetName() = 0;
        virtual wxImage* GetIcon() = 0;
        virtual void ShowSetup(wxWindow *parent) = 0;

        static wiPlugin* Init(wxString pluginName);
    protected:
        wxDynamicLibrary *m_lib;
};

#endif // WIPLUGIN_H

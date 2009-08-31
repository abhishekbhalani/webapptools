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
*//************************************************************
 * @file      wiApp.cpp
 * @brief     Code for Application Class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      30.06.2009
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include <wx/socket.h>
#include <wx/config.h>
#include <wx/log.h>
#include "wiApp.h"
#include "wiMainForm.h"

IMPLEMENT_APP(webInventApp);

bool webInventApp::OnInit()
{
    wxSocketBase::Initialize();
    SetAppName(wxT("WebInvent"));
    SetVendorName(wxT("[Lab18]"));
    wxConfig config(wxT("WebInvent"));
    // Localization
    {
        wxLogNull noLog;
        wxLocale::AddCatalogLookupPathPrefix(wxT("."));
        wxString fname;
        if (config.Read(wxT("language"), &fname)) {
            fname += wxT(".mo");
            m_locale.AddCatalog(fname);
        }
    }

    wiMainForm* frame = new wiMainForm(0L);

    frame->Show();

    return true;
}

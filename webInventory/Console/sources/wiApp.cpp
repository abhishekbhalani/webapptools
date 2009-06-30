/***************************************************************
 * Name:      wiApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Andrew "Stinger" Abramov (stinger@lab18.net)
 * Created:   2008-10-15
 * Copyright: Andrew "Stinger" Abramov (https://igate.lab18.net)
 * License:
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

//	m_statusBar1 = new wiStatBar( this );
//	SetStatusBar(m_statusBar1);

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

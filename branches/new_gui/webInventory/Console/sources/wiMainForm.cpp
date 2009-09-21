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
 * @file      wiMainForm.cpp
 * @brief     Code for MainForm class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      30.06.2009
 * @date      15.09.2009 - global refactoring
 **************************************************************/
#include "wiMainForm.h"

#include "../images/panReports.xpm"
#include "../images/panSettings.xpm"
#include "../images/panTasks.xpm"

#include "../images/webInventory.xpm"

#define wxPING_TIMER    999
#define wxPING_INTERVAL 10000

wiMainForm::wiMainForm( wxWindow* parent ) :
    MainForm( parent ),
    m_cfgEngine(APP_NAME)
{
    wxInitAllImageHandlers();

    SetIcon(wxICON(mainicon));

	m_pTasks = new wiTasks( m_mainnb ); //, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mainnb->AddPage( m_pTasks, _("Tasks"), false, wxBitmap( panTasks_xpm ) );
	m_pReports = new wiReports( m_mainnb ); //, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mainnb->AddPage( m_pReports, _("Reports"), false, wxBitmap( panReports_xpm ) );
	m_pSettings = new wiSettings( m_mainnb ); //, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mainnb->AddPage( m_pSettings, _("Settings"), true, wxBitmap( panSettings_xpm ) );

    m_statusBar = new wiStatBar( this );
    SetStatusBar(m_statusBar);

    m_client = NULL;
    m_plugList = NULL;
    m_connectionStatus = false;

    m_timer.SetOwner(this, wxPING_TIMER);
    this->Connect(wxEVT_TIMER, wxTimerEventHandler(wiMainForm::OnTimer));
    m_timer.Start(wxPING_INTERVAL);
}

void wiMainForm::OnClose( wxCloseEvent& event )
{
    /// @todo disconnect from server and save state
    m_cfgEngine.Write(wxT("language"), m_pSettings->GetLanguage());
    m_cfgEngine.Write(wxT("logging"), m_pSettings->GetLogging());

//    if (m_client != NULL) {
//        m_client->DoCmd(wxT("close"), wxT(""));
//    }

    m_cfgEngine.Flush();
    Destroy();
}

void wiMainForm::DoConnect(int account)
{
    wxString host, port;
    long idt;

    if (m_client != NULL) {
        // do disconnect first
        m_client->DoCmd(wxT("close"), wxT(""));
        m_pSettings->Disconnected();
        delete m_client;
        m_client = NULL;
    }
    m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Host"), account), &host);
    m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Port"), account), (int*)&idt);
    port = wxString::Format(wxT("%d"), idt);

    m_client = new wiTcpClient((char*)host.utf8_str().data(), (char*)port.utf8_str().data());
    m_client->Connect();
    host = m_client->GetScannerVersion();
    if (!host.IsEmpty()) {
        m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Name"), account), &m_connectionName);
        Connected();
        m_pSettings->SetServerVersion(host);
        LoadPluginList();
    }
    else {
        Disconnected();
        delete m_client;
        m_client = NULL;
    }
}

void wiMainForm::DoDisconnect()
{
    m_client->DoCmd(wxT("close"), wxT(""));
    m_pSettings->Disconnected();
    delete m_client;
    m_client = NULL;
}

wxString wiMainForm::DoClientCommand(const wxString& cmd, const wxString& params)
{
    if (m_client == NULL)
    {
        return wxT("");
    }
    return m_client->DoCmd(cmd, params);
}

char** wiMainForm::StringListToXpm(vector<string>& data)
{
    size_t lstSize = data.size();
    size_t i = 0;
    char** retval = new char*[lstSize];
    vector<string>::iterator line;

    for (line = data.begin(); line != data.end(); line++) {
        retval[i++] = strdup((*line).c_str());
    }
    return retval;
}

wxString wiMainForm::FromStdString(const std::string& str)
{
    wxString retval = wxString::FromUTF8(str.c_str());
    return retval;
}

void wiMainForm::LoadPluginList()
{
    if (m_plugList != NULL) {
        delete m_plugList;
        m_plugList = NULL;
    }
    if (m_client != NULL) {
        m_plugList = m_client->GetPluginList();
        m_pSettings->ShowPluginsList(m_plugList);
    }
}

void wiMainForm::Connected(bool forced)
{
    m_pTasks->Connected();
    m_pSettings->Connected();
    m_statusBar->SetImage(wiSTATUS_BAR_YES);
    m_statusBar->SetStatusText(m_connectionName, 2);
}

void wiMainForm::Disconnected(bool forced)
{
    m_pTasks->Disconnected(forced);
    m_pSettings->Disconnected(forced);
    m_statusBar->SetImage(wiSTATUS_BAR_NO);
    m_connectionName = wxT("");
    m_statusBar->SetStatusText(m_connectionName, 2);
}

void wiMainForm::OnTimer( wxTimerEvent& event )
{
    /// @todo set status text
    if (m_client != NULL) {
        if (m_client->Ping()) {
            if (m_connectionStatus == false) {
                Connected(false);
                wxString vers;
                vers = m_client->GetScannerVersion();
                if (!vers.IsEmpty()) {
                    m_pSettings->SetLabel(vers);
                }
            }
//            ProcessTaskList(wxT(""));
//            ProcessObjects(wxT(""));
//            ProcessProfileList(wxT(""));
            m_connectionStatus = true;
        }
        else {
            if (m_connectionStatus == true) {
                Disconnected(false);
                m_statusBar->SetImage(wiSTATUS_BAR_NO);
//                m_stSrvVersData->SetLabel(_("unknown"));
            }
            m_connectionStatus = false;
        }
    }
    else {
        m_statusBar->SetImage(wiSTATUS_BAR_UNK);
        m_statusBar->SetStatusText(wxT(""), 3);
        m_connectionStatus = false;
    }
}

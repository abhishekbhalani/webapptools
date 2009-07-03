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
 **************************************************************/
#include <wx/filename.h>
#include "wiStatBar.h"
#include "wiServDialog.h"
#include "wiMainForm.h"
#include "wxThings.h"
#include "version.h"
#include "../images/webInventory.xpm"
#include "../images/btnStop.xpm"
#include "../images/start.xpm"

#define wxPING_TIMER    999
#define wxPING_INTERVAL 10000

//    m_statusBar = new wiStatBar( this );
//    SetStatusBar(m_statusBar);
//    m_btnApply = new wxCustomButton( m_pTasks, wxID_ANY, _("Apply"), wxBitmap( apply_xpm ), wxDefaultPosition, wxDefaultSize, wxCUSTBUT_BUTTON|wxCUSTBUT_RIGHT);

wiMainForm::wiMainForm( wxWindow* parent ) :
        MainForm( parent ),
        m_cfgEngine(wxT("WebInvent")),
        m_client(NULL),
        m_timer(this, 1)
{
    connStatus = true;
    wxInitAllImageHandlers();
    SetIcon(wxICON(mainicon));

    wxString fname;
    wxFileName fPath;

    fname = wxFindFirstFile(wxT("*.mo"), wxFILE);
    while (!fname.IsEmpty()) {
        fPath.Assign(fname);
        fname = fPath.GetName();
        //! @todo: Capitalise first char
        m_chLangs->AppendString(fname);
        fname = wxFindNextFile();
    }
    m_chLangs->SetSelection(0);

    wxString cfgData;
    int iData;
    iData = 0;
    if (m_cfgEngine.Read(wxT("language"), &cfgData)) {
        iData = m_chLangs->FindString(cfgData);
    }
    m_chLangs->SetSelection(iData);

    m_lstActiveTask->InsertColumn(0, wxT(""), wxLIST_FORMAT_LEFT, 18);
    m_lstActiveTask->InsertColumn(1, _("Task name"), wxLIST_FORMAT_LEFT, 300);
    m_lstActiveTask->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, 77);

    m_lstTaskList->InsertColumn(0, wxT(""), wxLIST_FORMAT_LEFT, 18);
    m_lstTaskList->InsertColumn(1, _("Task name"), wxLIST_FORMAT_LEFT, 300);
    m_lstTaskList->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, 77);

    wxString label;
    label = wxString::FromAscii(AutoVersion::FULLVERSION_STRING) + wxT(" ");
    label += wxString::FromAscii(AutoVersion::STATUS);
#if defined(__WXMSW__)
        label << _T(" (Windows)");
#elif defined(__WXMAC__)
        label << _T(" (Mac)");
#elif defined(__UNIX__)
        label << _T(" (Linux)");
#endif
    m_stConVersData->SetLabel(label);
    m_statusBar->SetStatusText(_("Disconnected"), 1);
    m_statusBar->SetStatusText(_("unknown"), 2);
    LoadConnections();
    m_timer.SetOwner(this, wxPING_TIMER);
    this->Connect(wxEVT_TIMER, wxTimerEventHandler(wiMainForm::OnTimer));
    m_timer.Start(wxPING_INTERVAL);


}

void wiMainForm::OnTimer( wxTimerEvent& event )
{
    /// @todo set status text
    if (m_client != NULL) {
        if (m_client->Ping()) {
            m_statusBar->SetImage(wiSTATUS_BAR_YES);
            m_statusBar->SetStatusText(_("Connected"), 1);
            m_statusBar->SetStatusText(wxT(""), 3);
            if (connStatus == false) {
                wxString vers;
                vers = m_client->GetScannerVersion();
                if (!vers.IsEmpty()) {
                    m_stSrvVersData->SetLabel(vers);
                }
            }
            connStatus = true;
        }
        else {
            m_statusBar->SetImage(wiSTATUS_BAR_NO);
            m_statusBar->SetStatusText(_("Disconnected"), 1);
            m_statusBar->SetStatusText(m_client->GetLastError(), 3);
            m_stSrvVersData->SetLabel(_("unknown"));
            connStatus = false;
        }
    }
    else {
        m_statusBar->SetImage(wiSTATUS_BAR_UNK);
        m_statusBar->SetStatusText(wxT(""), 3);
        connStatus = false;
    }
}

void wiMainForm::OnClose( wxCloseEvent& event )
{
    // @todo disconnect from server and save state
    m_cfgEngine.Write(wxT("language"), m_chLangs->GetString(m_chLangs->GetSelection()));

    if (m_client != NULL) {
        m_client->DoCmd(wxT("close"), wxT(""));
    }

    m_cfgEngine.Flush();
    Destroy();
}

void wiMainForm::OnLangChange( wxCommandEvent& event )
{
    m_cfgEngine.Write(wxT("language"), m_chLangs->GetString(m_chLangs->GetSelection()));
    m_stLangRestart->Show();
    m_pSettings->Layout();
}

void wiMainForm::OnConnect( wxCommandEvent& event )
{
    wxString host, port;
    int idx;
    long idt;

    if (m_client != NULL) {
        // close current connection
        m_client->DoCmd(wxT("close"), wxT(""));
        m_statusBar->SetImage(wiSTATUS_BAR_UNK);
        m_statusBar->SetStatusText(_("Disconnected"), 1);
        m_statusBar->SetStatusText(_("unknown"), 2);
        m_bpConnect->SetToolTip(_("Connect"));
        m_bpConnect->SetBitmapLabel(wxBitmap(start_xpm));
        delete m_client;
        m_client = NULL;
        return;
    }
    idx = m_chServers->GetSelection();
    if (idx > -1) {
        wxString label = m_chServers->GetString(idx);
        if (label != wxT("hostname[:port]")) {
            m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Host"), idx), &host);
            m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Port"), idx), (int*)&idt);
            port = wxString::Format(wxT("%d"), idt);

            m_client = new wiTcpClient(host.ToAscii().data(), port.ToAscii().data());
            m_client->Connect();
            host = m_client->GetScannerVersion();
            if (!host.IsEmpty()) {
                m_stSrvVersData->SetLabel(host);
                m_statusBar->SetImage(wiSTATUS_BAR_YES);
                m_statusBar->SetStatusText(_("Connected"), 1);
                m_statusBar->SetStatusText(label, 2);
                m_statusBar->SetStatusText(wxT(""), 3);
                m_bpConnect->SetToolTip(_("Disconnect"));
                m_bpConnect->SetBitmapLabel(wxBitmap(btnStop_xpm));
            }
            else {
                m_statusBar->SetImage(wiSTATUS_BAR_NO);
                m_statusBar->SetStatusText(_("Disconnected"), 1);
                m_statusBar->SetStatusText(_("unknown"), 2);
                m_statusBar->SetStatusText(m_client->GetLastError(), 3);
                m_bpConnect->SetToolTip(_("Connect"));
                m_bpConnect->SetBitmapLabel(wxBitmap(start_xpm));
                delete m_client;
                m_client = NULL;
            }
        }
    }
}

void wiMainForm::OnAddServer( wxCommandEvent& event )
{
    wiServDialog srvDlg(this);
    wxString name, host, port;
    int idx;
    long idt;

    if (srvDlg.ShowModal() == wxOK) {
        name = srvDlg.m_txtName->GetValue();
        host = srvDlg.m_txtHostname->GetValue();
        port = srvDlg.m_txtSrvPort->GetValue();
        if (m_cfgEngine.GetAccountIndex(name) == -1) {
            if (port.ToLong(&idt) && (idt > 0 && idt < 65536)) {
                idx = m_chServers->Append(name);
                m_cfgEngine.Write(wxString::Format(wxT("Connection%d/Name"), idx), name);
                m_cfgEngine.Write(wxString::Format(wxT("Connection%d/Host"), idx), host);
                m_cfgEngine.Write(wxString::Format(wxT("Connection%d/Port"), idx), idt);
            }
            else {
                wxMessageBox(_("Port value is out of range"), wxT("WebInvent"), wxICON_ERROR | wxOK, this);
            }
        }
        else {
            wxMessageBox(_("Connection with the same name already exist"), wxT("WebInvent"), wxICON_ERROR | wxOK, this);
        }
    }
}

void wiMainForm::OnEditServer( wxCommandEvent& event )
{
    wiServDialog srvDlg(this);
    wxString name, host, port;
    int idx;
    long idt;

    idx = m_chServers->GetSelection();
    if (idx > -1) {
        m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Name"), idx), &name);
        m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Host"), idx), &host);
        m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Port"), idx), (int*)&idt);
        port = wxString::Format(wxT("%d"), idt);
        srvDlg.m_txtName->SetValue(name);
        srvDlg.m_txtHostname->SetValue(host);
        srvDlg.m_txtSrvPort->SetValue(port);
        if (srvDlg.ShowModal() == wxOK) {
            name = srvDlg.m_txtName->GetValue();
            host = srvDlg.m_txtHostname->GetValue();
            port = srvDlg.m_txtSrvPort->GetValue();
            if (port.ToLong(&idt) && (idt > 0 && idt < 65536)) {
                m_chServers->SetString(idx, name);
                m_cfgEngine.Write(wxString::Format(wxT("Connection%d/Name"), idx), name);
                m_cfgEngine.Write(wxString::Format(wxT("Connection%d/Host"), idx), host);
                m_cfgEngine.Write(wxString::Format(wxT("Connection%d/Port"), idx), idt);
            }
            else {
                wxMessageBox(_("Port value is out of range"), wxT("WebInvent"), wxICON_ERROR | wxOK, this);
            }
        }
    }
}

void wiMainForm::OnDelServer( wxCommandEvent& event )
{
    int res = -1;
    int idx = -1;
    wxString data;

    idx = m_chServers->GetSelection();
    if (idx > -1) {
        /// @todo check connection state and disconnect if needed
        wxString label = m_chServers->GetString(m_chServers->GetSelection());
        if (label != wxT("hostname[:port]")) {
            wxString msg = wxString::Format(_("Are you shure to delete the connection '%s'?"), label.c_str());
            res = wxMessageBox(msg, _("Confirm"), wxYES_NO | wxICON_QUESTION, this);
            if (res == wxYES) {
                m_chServers->Delete(idx);
                while (m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Name"), (idx+1)), &data)) {
                    m_cfgEngine.CopyAccount(idx+1, idx);
                    idx++;
                }
                m_cfgEngine.DeleteGroup(wxString::Format(wxT("Connection%d"), idx));
                m_chServers->SetSelection(0);
            }
        }
    }
}

void wiMainForm::LoadConnections()
{
    int res = 0;
    wxString data;

    m_chServers->Clear();
    while (m_cfgEngine.Read(wxString::Format(wxT("Connection%d/Name"), res), &data)) {
        res++;
        m_chServers->Append(data);
    }
    m_chServers->SetSelection(-1);
}


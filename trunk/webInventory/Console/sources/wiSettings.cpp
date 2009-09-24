#include <wx/dcmemory.h>
#include "wiSettings.h"
#include "wiMainForm.h"
#include "wiServDialog.h"
#include "version.h"

#include "../images/_btnAdd.xpm"
#include "../images/_btnApply16.xpm"
#include "../images/_btnDel.xpm"
#include "../images/_btnEdit.xpm"
#include "../images/_opts_audit.xpm"
#include "../images/_reload.xpm"
#include "../images/_refresh.xpm"
#include "../images/_start.xpm"
#include "../images/_tree_no.xpm"

#include "../images/start.xpm"
#include "../images/btnStop.xpm"

wiSettings::wiSettings( wxWindow* parent )
:
Settings( parent )
{
    wxString fname;
    wxString label;
    wxFileName fPath;
    int iData;

    // init plugins list
    InitPluginsGrid();

    // fix tolbar images - add "disabled"
    m_toolBarSrv->SetToolDisabledBitmap(wxID_TLCONNECT, wxBitmap(_start_xpm));
    m_toolBarSrv->SetToolDisabledBitmap(wxID_TLNEW, wxBitmap(_btnAdd_xpm));
    m_toolBarSrv->SetToolDisabledBitmap(wxID_TLEDIT, wxBitmap(_btnEdit_xpm));
    m_toolBarSrv->SetToolDisabledBitmap(wxID_TLDELETE, wxBitmap(_btnDel_xpm));

    m_toolBarLang->SetToolDisabledBitmap(wxID_TLLANGAPPLY, wxBitmap(_btnApply16_xpm));

    m_tbServer->SetToolDisabledBitmap(wxID_TLPLGRELOAD, wxBitmap(_refresh_xpm));
    m_tbServer->SetToolDisabledBitmap(wxID_TLSRVRESTART, wxBitmap(_reload_xpm));
    m_tbServer->SetToolDisabledBitmap(wxID_TLSRVSTOP, wxBitmap(_tree_no_xpm));
    m_tbServer->SetToolDisabledBitmap(wxID_TLSRVLOG, wxBitmap(_opts_audit_xpm));

    // set version information
    label = wxString::FromAscii(AutoVersion::FULLVERSION_STRING) + wxT(" ");
    label += wxString::FromAscii(AutoVersion::STATUS);
#if defined(__WXMSW__)
    label += _T(" (Windows)");
#elif defined(__WXMAC__)
    label += _T(" (Mac)");
#elif defined(__UNIX__)
    label += _T(" (Linux)");
#endif
#ifdef __WXDEBUG__
    label += wxString::Format(wxT(" SVN build #%d"), AutoVersion::BUILDS_COUNT);
#endif //__WXDEBUG__
    m_stConVersData->SetLabel(label);

    // load language files
    fname = wxFindFirstFile(wxT("*.mo"), wxFILE);
    while (!fname.IsEmpty()) {
        fPath.Assign(fname);
        fname = fPath.GetName();
        //! @todo: Capitalise first char
        m_chLangs->AppendString(fname);
        fname = wxFindNextFile();
    }
    m_chLangs->SetSelection(0);
    if (FRAME_WINDOW->Config()->Read(wxT("language"), &fname)) {
        iData = m_chLangs->FindString(fname);
    }
    m_chLangs->SetSelection(iData);

    iData = 0;
    if (FRAME_WINDOW->Config()->Read(wxT("logging"), &iData)) {
        m_chLogging->SetValue(iData);
    }

    m_pnServer->Hide();
    LoadConnections();

    m_toolBarSrv->EnableTool(wxID_TLCONNECT, false);
    m_toolBarSrv->EnableTool(wxID_TLEDIT, false);
    m_toolBarSrv->EnableTool(wxID_TLDELETE, false);

    connected = false;
}

void wiSettings::LoadConnections()
{
    int res = 0;
    wxString data;

    m_chServers->Clear();
    while (FRAME_WINDOW->Config()->Read(wxString::Format(wxT("Connection%d/Name"), res), &data)) {
        res++;
        m_chServers->Append(data);
    }
    m_chServers->SetSelection(-1);
}

void wiSettings::OnChangeSrv( wxCommandEvent& event )
{
    if (event.GetSelection() > -1) {
        m_toolBarSrv->EnableTool(wxID_TLCONNECT, true);
        m_toolBarSrv->EnableTool(wxID_TLEDIT, true);
        m_toolBarSrv->EnableTool(wxID_TLDELETE, true);
    }
}

void wiSettings::InitPluginsGrid()
{
    wxPropertyGridPage* pg;

    m_pgrdManager->Clear();
    m_pgrdManager->AddPage(wxT("PluginInfo"));

    pg = m_pgrdManager->GetPage(wxT("PluginInfo"));
    pg->Append( new wxPropertyCategory(_("Generic plugins"), wxT("generic")));
    pg->Append( new wxPropertyCategory(_("Storage plugins"), wxT("storage")));
    pg->Append( new wxPropertyCategory(_("Transport plugins"), wxT("transport")));
    pg->Append( new wxPropertyCategory(_("Inventory plugins"), wxT("inventory")));
    pg->Append( new wxPropertyCategory(_("Audit plugins"), wxT("audit")));
    pg->Append( new wxPropertyCategory(_("Vulnerabilities detection plugins"), wxT("vulner")));

    fgSrvSizer->Layout();
    m_pgrdManager->CollapseAll();
    m_pgrdManager->ExpandAll();
}

void wiSettings::OnConnect( wxCommandEvent& event )
{
    if (connected) {
        // invoke disconnect
        FRAME_WINDOW->DoDisconnect();
    }
    else {
        // connect to server
        int idx = m_chServers->GetSelection();
        if (idx > -1) {
            FRAME_WINDOW->DoConnect(idx);
        }
    }
}

void wiSettings::OnAddServer( wxCommandEvent& event )
{
    wiServDialog srvDlg(this);
    wxString name, host, port;
    int idx;
    long idt;

    if (srvDlg.ShowModal() == wxOK) {
        name = srvDlg.m_txtName->GetValue();
        host = srvDlg.m_txtHostname->GetValue();
        port = srvDlg.m_txtSrvPort->GetValue();
        if (FRAME_WINDOW->Config()->GetAccountIndex(name) == -1) {
            if (port.ToLong(&idt) && (idt > 0 && idt < 65536)) {
                idx = m_chServers->Append(name);
                FRAME_WINDOW->Config()->Write(wxString::Format(wxT("Connection%d/Name"), idx), name);
                FRAME_WINDOW->Config()->Write(wxString::Format(wxT("Connection%d/Host"), idx), host);
                FRAME_WINDOW->Config()->Write(wxString::Format(wxT("Connection%d/Port"), idx), (int)idt);
            }
            else {
                wxMessageBox(_("Port value is out of range"), APP_TITLE, wxICON_ERROR | wxOK, this);
            }
        }
        else {
            wxMessageBox(_("Connection with the same name already exist"), APP_TITLE, wxICON_ERROR | wxOK, this);
        }
    }
}

void wiSettings::OnEditServer( wxCommandEvent& event )
{
    wiServDialog srvDlg(this);
    wxString name, host, port;
    int idx;
    long idt;

    idx = m_chServers->GetSelection();
    if (idx > -1) {
        FRAME_WINDOW->Config()->Read(wxString::Format(wxT("Connection%d/Name"), idx), &name);
        FRAME_WINDOW->Config()->Read(wxString::Format(wxT("Connection%d/Host"), idx), &host);
        FRAME_WINDOW->Config()->Read(wxString::Format(wxT("Connection%d/Port"), idx), (int*)&idt);
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
                FRAME_WINDOW->Config()->Write(wxString::Format(wxT("Connection%d/Name"), idx), name);
                FRAME_WINDOW->Config()->Write(wxString::Format(wxT("Connection%d/Host"), idx), host);
                FRAME_WINDOW->Config()->Write(wxString::Format(wxT("Connection%d/Port"), idx), (int)idt);
            }
            else {
                wxMessageBox(_("Port value is out of range"), APP_TITLE, wxICON_ERROR | wxOK, this);
            }
        }
    }
}

void wiSettings::OnDelServer( wxCommandEvent& event )
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
            res = wxMessageBox(msg, APP_TITLE, wxYES_NO | wxICON_QUESTION, this);
            if (res == wxYES) {
                m_chServers->Delete(idx);
                while (FRAME_WINDOW->Config()->Read(wxString::Format(wxT("Connection%d/Name"), (idx+1)), &data)) {
                    FRAME_WINDOW->Config()->CopyAccount(idx+1, idx);
                    idx++;
                }
                FRAME_WINDOW->Config()->DeleteGroup(wxString::Format(wxT("Connection%d"), idx));
                m_chServers->SetSelection(0);
            }
        }
    }
}

void wiSettings::OnLangChange( wxCommandEvent& event )
{
    FRAME_WINDOW->Config()->Write(wxT("language"), m_chLangs->GetString(m_chLangs->GetSelection()));
    m_stLangRestart->Show();
    Layout();
}

void wiSettings::OnStorageChange( wxCommandEvent& event )
{
    PluginList* plugList;
    int plg;

    plugList = FRAME_WINDOW->GetPluginList();
    plg = m_chStorage->GetSelection();
    if (plg != -1 && plugList != NULL && FRAME_WINDOW->IsConnected()) {
        int plgIdx = (int)m_chStorage->GetClientData(plg) - wxPluginsData;
        if (plgIdx >= 0 && plgIdx < plugList->size()) {
            wxString plgID = FRAME_WINDOW->FromStdString((*plugList)[plgIdx].PluginId);
            FRAME_WINDOW->DoClientCommand(wxT("setstorage"), plgID);
            FRAME_WINDOW->Connected(true);
        }
    }
}

void wiSettings::OnPlgRefresh( wxCommandEvent& event )
{
	FRAME_WINDOW->LoadPluginList();
}

void wiSettings::OnSrvRestart( wxCommandEvent& event )
{
    int res = wxMessageBox(_("Are you shure to restart the server?\n(All active tasks will be paused, and all clients will be disconnected)"),
            _("Warning"), wxICON_QUESTION | wxYES_NO);
    if (res == wxYES) {
        FRAME_WINDOW->DoClientCommand(wxT("restart"), wxT(""));
    }
}

void wiSettings::OnSrvStop( wxCommandEvent& event )
{
    int res = wxMessageBox(_("Are you shure to stop the server?\n(All active tasks will be paused, and all clients will be disconnected)"),
            _("Warning"), wxICON_QUESTION | wxYES_NO);
    if (res == wxYES) {
        FRAME_WINDOW->DoClientCommand(wxT("exit"), wxT(""));
    }
}

void wiSettings::OnServerLogs( wxCommandEvent& event )
{
    wxMessageBox(wxT("Coming soon!"), _("Information"), wxICON_INFORMATION | wxOK);
}

void wiSettings::Connected(bool forced)
{
    connected = true;
    m_pnServer->Show();
    m_pnServer->Enable();
	m_chServers->Disable();
    if (forced) {
        m_toolBarSrv->SetToolNormalBitmap(wxID_TLCONNECT, wxBitmap(btnStop_xpm));
        m_toolBarSrv->SetToolShortHelp(wxID_TLCONNECT, _("Disconnect"));
        m_toolBarSrv->EnableTool(wxID_TLEDIT, false);
        m_toolBarSrv->EnableTool(wxID_TLDELETE, false);
    }
}

void wiSettings::Disconnected(bool forced)
{
    connected = false;
    if (forced) {
        m_pnServer->Hide();
        m_chServers->Enable();
        m_toolBarSrv->SetToolNormalBitmap(wxID_TLCONNECT, wxBitmap(start_xpm));
        m_toolBarSrv->SetToolShortHelp(wxID_TLCONNECT, _("Connect"));
        m_toolBarSrv->EnableTool(wxID_TLEDIT, true);
        m_toolBarSrv->EnableTool(wxID_TLDELETE, true);
    }
    else {
        m_pnServer->Disable();
    }
}

wxString wiSettings::GetLanguage()
{
    return m_chLangs->GetString(m_chLangs->GetSelection());
}

bool wiSettings::GetLogging()
{
    return m_chLogging->GetValue();
}

void wiSettings::SetServerVersion(const wxString& ver)
{
    m_stSrvVersData->SetLabel(ver);
}

void wiSettings::SetStorage(const wxString& plg)
{
    int pos;
    PluginList* plugList;

    plugList = FRAME_WINDOW->GetPluginList();
    if (plugList != NULL) {
        int index;
        for (pos = 0; pos < m_chStorage->GetCount(); pos++) {
            index = (int)(m_chStorage->GetClientData(pos)) - wxPluginsData;
            if (index >= 0 && index < plugList->size()) {
                wxString id = FRAME_WINDOW->FromStdString((*plugList)[index].PluginId);
                if (plg.CmpNoCase(id) == 0) {
                    m_chStorage->SetSelection(pos);
                }
            }
        }
    }
}

void wiSettings::ShowPluginsList(PluginList* plugList)
{
    int i;
    size_t lstSize;
    wxString label, group, ifn;
    wxArrayString ifaces;
    wxPropertyGridPage* pg;
    wxBitmap* bmp;
    wxSize bmpSize;
    wxMemoryDC mdc;
    int x, y, h, w;

    m_pgrdManager->Freeze();
    m_chStorage->Freeze();
    m_chStorage->Clear();

    InitPluginsGrid();

    bmpSize = m_pgrdManager->GetGrid()->GetImageSize();
    w = bmpSize.GetWidth();
    h = bmpSize.GetHeight() ;
    x = w - 16;
    x /= 2;
    if (x > 0) {
        x = 0;
    }
    y = h - 16;
    y /= 2;
    if (y > 0) {
        y = 0;
    }

    pg = m_pgrdManager->GetPage(wxT("PluginInfo"));

    if (plugList != NULL) {
        for (lstSize = 0; lstSize < plugList->size(); lstSize++) {
            ifaces.Clear();
            group = wxT("generic");

            label = FRAME_WINDOW->FromStdString((*plugList)[lstSize].PluginDesc);

            for (i = 0; i < (*plugList)[lstSize].IfaceList.size(); i++)
            {
                ifaces.Insert(FRAME_WINDOW->FromStdString((*plugList)[lstSize].IfaceList[i]), 0);
            }

            char** xpm = FRAME_WINDOW->StringListToXpm((*plugList)[lstSize].PluginIcon);
            wxBitmap *ico = new wxBitmap(xpm);

            // add plugins to the storage list
            if (find((*plugList)[lstSize].IfaceList.begin(), (*plugList)[lstSize].IfaceList.end(), "iStorage") != (*plugList)[lstSize].IfaceList.end()) {
                group = wxT("storage");
                int itm = m_chStorage->Append(FRAME_WINDOW->FromStdString((*plugList)[lstSize].PluginDesc), (void*)(wxPluginsData + lstSize));
//                if (storageID.CmpNoCase(FromStdString((*m_plugList)[lstSize].PluginId)) == 0) {
//                    m_chStorage->SetSelection(itm);
//                }
            }
            if (find((*plugList)[lstSize].IfaceList.begin(), (*plugList)[lstSize].IfaceList.end(), "iTransport") != (*plugList)[lstSize].IfaceList.end()) {
                group = wxT("transport");
            }
            if (find((*plugList)[lstSize].IfaceList.begin(), (*plugList)[lstSize].IfaceList.end(), "iAudit") != (*plugList)[lstSize].IfaceList.end()) {
                group = wxT("audit");
            }
            if (find((*plugList)[lstSize].IfaceList.begin(), (*plugList)[lstSize].IfaceList.end(), "iVulner") != (*plugList)[lstSize].IfaceList.end()) {
                group = wxT("vulner");
            }
            if (find((*plugList)[lstSize].IfaceList.begin(), (*plugList)[lstSize].IfaceList.end(), "iInventory") != (*plugList)[lstSize].IfaceList.end()) {
                group = wxT("inventory");
            }

            wxPGProperty* propId = pg->Insert(group, -1, new wxEnumProperty(label, wxPG_LABEL, ifaces) );

            bmp = new wxBitmap(w, h, 32);
            mdc.SelectObject(*bmp);
            mdc.Clear();
            mdc.DrawBitmap(xpm, x, y, true);
            mdc.SelectObject(wxNullBitmap);
            pg->SetPropertyImage( propId, *bmp );
        }
    }
    m_chStorage->Thaw();
    m_pgrdManager->Thaw();
    Layout();
}

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
#include <wx/wupdlock.h>
#include <wx/xrc/xmlres.h>
#include <wx/fs_mem.h>
#include "wiStatBar.h"
#include "wiServDialog.h"
#include "wiMainForm.h"
#include "wxThings.h"
#include "version.h"
#include "../images/webInventory.xpm"
#include "../images/btnStop.xpm"
#include "../images/start.xpm"
#include "../images/pause.xpm"

/// @todo change this
#include "../images/tree_no.xpm"
#include "../images/tree_unk.xpm"
#include "../images/tree_yes.xpm"

#define wxPING_TIMER    999
#define wxPING_INTERVAL 10000

//    m_statusBar = new wiStatBar( this );
//    SetStatusBar(m_statusBar);
//    m_btnApply = new wxCustomButton( m_panTaskOpts, wxID_ANY, _("Apply"), wxBitmap( apply_xpm ), wxDefaultPosition, wxDefaultSize, wxCUSTBUT_BUTTON|wxCUSTBUT_RIGHT);

static const wxChar* gTaskStatus[] = {_("idle"),
                                      _("run (%d%%)"),
                                      _("paused (%d%%)")};

static int sortOrder = 0; // for now - the boolean order: 0 - normal, -1 - reverse
static int wxCALLBACK SortItemFunc(long item1, long item2, long ctrlPtr)
{
    wxListCtrl* ctrl = (wxListCtrl*)ctrlPtr;
    wxListItem info1;
    wxListItem info2;
    long idx1, idx2;
    int retVal;

    retVal = 0;
    if (ctrl) {
        idx1 = ctrl->FindItem(-1, item1);
        idx2 = ctrl->FindItem(-1, item2);
        if (idx1 == -1) {
            return 1;
        }
        if (idx2 == -1) {
            return -1;
        }
        info1.SetId(idx1);
        info1.SetColumn(0);
        info1.SetMask(wxLIST_MASK_IMAGE);
        ctrl->GetItem(info1);
        idx1 = info1.GetImage();
        if ( idx1 == 0) {
            idx1 = 0xFFFFFF;
        }

        info2.SetId(idx2);
        info2.SetColumn(0);
        info2.SetMask(wxLIST_MASK_IMAGE);
        ctrl->GetItem(info2);
        idx2 = info2.GetImage();
        if ( idx2 == 0) {
            idx2 = 0xFFFFFF;
        }

        if ( idx1 < idx2 ) {
            retVal = -1;
        }
        if ( idx1 > idx2 ) {
            retVal = 1;
        }
        if ( sortOrder ) {
            retVal *= -1;
        }
    }
    return retVal;
}

static char** StringListToXpm(vector<string>& data)
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

wiMainForm::wiMainForm( wxWindow* parent ) :
        MainForm( parent ),
        m_cfgEngine(wxT("WebInvent")),
        m_client(NULL),
        m_timer(this, 1),
        m_lstImages(16, 16)
{
    connStatus = true;
    wxInitAllImageHandlers();
    wxFileSystem::AddHandler(new wxMemoryFSHandler);
    wxXmlResource::Get()->InitAllHandlers();

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

    m_lstImages.Add(wxIcon(tree_unk));
    m_lstImages.Add(wxIcon(tree_yes));
    m_lstImages.Add(wxIcon(tree_no));

    m_lstTaskList->InsertColumn(0, wxT(""), wxLIST_FORMAT_LEFT, 20);
    m_lstTaskList->InsertColumn(1, _("Task name"), wxLIST_FORMAT_LEFT, 180);
    m_lstTaskList->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, 96);
    m_lstTaskList->SetImageList(&m_lstImages, wxIMAGE_LIST_SMALL);

    m_lstTaskList->DeleteAllItems();

    m_plugins = 0;
    m_plugList = NULL;

    wxString label;
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
    m_statusBar->SetStatusText(_("Disconnected"), 1);
    m_statusBar->SetStatusText(_("unknown"), 2);
    LoadConnections();
    m_timer.SetOwner(this, wxPING_TIMER);
    this->Connect(wxEVT_TIMER, wxTimerEventHandler(wiMainForm::OnTimer));
    m_timer.Start(wxPING_INTERVAL);
    m_panTaskOpts->Disable();
    SelectTask( -1 );
    m_pnServer->Hide();
    Layout();
}

void wiMainForm::OnTimer( wxTimerEvent& event )
{
    /// @todo set status text
    if (m_client != NULL) {
        if (m_client->Ping()) {
            Connected(false);
            if (connStatus == false) {
                wxString vers;
                vers = m_client->GetScannerVersion();
                if (!vers.IsEmpty()) {
                    m_stSrvVersData->SetLabel(vers);
                }
            }
            ProcessTaskList(wxT(""));
            connStatus = true;
        }
        else {
            Disconnected(false);
            m_statusBar->SetImage(wiSTATUS_BAR_NO);
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
        Disconnected();
        delete m_client;
        m_client = NULL;
        m_lstTaskList->DeleteAllItems();
        SelectTask( -1 );
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
                Connected();
                GetPluginList();
                m_stSrvVersData->SetLabel(host);
                m_statusBar->SetStatusText(label, 2);
            }
            else {
                Disconnected();
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
                m_cfgEngine.Write(wxString::Format(wxT("Connection%d/Port"), idx), (int)idt);
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
                m_cfgEngine.Write(wxString::Format(wxT("Connection%d/Port"), idx), (int)idt);
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

void wiMainForm::ProcessTaskList(const wxString& criteria/* = wxT("")*/)
{
    TaskList* lst;
    size_t lstSize;
    int idx = 0;
    long idLong;

    if(m_client != NULL) {
        lst = m_client->GetTaskList(criteria);
        if (lst != NULL) {
            wxWindowUpdateLocker taskList(m_lstTaskList);

            m_lstTaskList->DeleteAllItems();
            for (lstSize = 0; lstSize < lst->size(); lstSize++) {
                wxString idStr = wxString::FromAscii((*lst)[lstSize].id.c_str());
                idStr.ToLong(&idLong, 16);
                idx = m_lstTaskList->GetItemCount();
                m_lstTaskList->InsertItem(idx, (*lst)[lstSize].status);
                m_lstTaskList->SetItem(idx, 1, wxString::FromAscii((*lst)[lstSize].name.c_str()));
                if ((*lst)[lstSize].status >= 0 && (*lst)[lstSize].status < WI_TSK_MAX) {
                    m_lstTaskList->SetItem(idx, 2, wxString::Format(gTaskStatus[(*lst)[lstSize].status], (*lst)[lstSize].completion));
                }
                m_lstTaskList->SetItemData(idx, idLong);
            }
            if (m_selectedTask >= m_lstTaskList->GetItemCount()) {
                m_selectedTask = m_lstTaskList->GetItemCount() - 1;
            }
            m_lstTaskList->SortItems(SortItemFunc, (long)m_lstTaskList);
            m_lstTaskList->SetItemState(m_selectedTask, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
        else {
            m_statusBar->SetImage(wiSTATUS_BAR_NO);
            m_statusBar->SetStatusText(m_client->GetLastError(), 3);
        }
    }
}

void wiMainForm::Disconnected(bool mode)
{
    m_bpTaskNew->Disable();
    m_pnServer->Disable();
    m_statusBar->SetImage(wiSTATUS_BAR_NO);
    m_statusBar->SetStatusText(_("Disconnected"), 1);
    m_statusBar->SetStatusText(_("unknown"), 2);
    m_statusBar->SetStatusText(m_client->GetLastError(), 3);
    if (mode) {
        m_bpConnect->SetToolTip(_("Connect"));
        m_bpConnect->SetBitmapLabel(wxBitmap(start_xpm));
        m_pnServer->Hide();
        Layout();
    }
}

void wiMainForm::Connected(bool mode)
{
    m_bpTaskNew->Enable();
    m_pnServer->Enable();
    m_statusBar->SetImage(wiSTATUS_BAR_YES);
    m_statusBar->SetStatusText(_("Connected"), 1);
    m_statusBar->SetStatusText(wxT(""), 3);
    if (mode) {
        m_bpConnect->SetToolTip(_("Disconnect"));
        m_bpConnect->SetBitmapLabel(wxBitmap(btnStop_xpm));
        ProcessTaskList();
        m_pnServer->Show();
        Layout();
    }
}

void wiMainForm::OnAddTask( wxCommandEvent& event )
{
    wxString name;

    name = wxGetTextFromUser(_("Input new task name"), _("Query"), wxT(""), this);
    if (!name.IsEmpty() && m_client != NULL) {
        name = m_client->DoCmd(wxT("addtask"), name);
    }
    ProcessTaskList();
}

void wiMainForm::OnDelTask( wxCommandEvent& event )
{
    wxString name;
    wxListItem info;

    if (m_selectedTask > -1)
    {
        info.SetId(m_selectedTask);
        info.SetColumn(1);
        info.SetMask(wxLIST_MASK_TEXT);
        m_lstTaskList->GetItem(info);
        name = wxString::Format(_("Are you sure to delete task '%s'?"), info.GetText());
        int res = wxMessageBox(name, _("Confirm"), wxYES_NO | wxICON_QUESTION, this);
        if (res == wxYES) {
            int id = m_lstTaskList->GetItemData(m_selectedTask);
            name = wxString::Format(wxT("%X"), id);
            m_client->DoCmd(wxT("deltask"), name);

            ProcessTaskList();
        }
    }
}

void wiMainForm::OnRunTask( wxCommandEvent& event )
{
    wxListItem info;

    if (m_selectedTask > -1)
    {
        int id = m_lstTaskList->GetItemData(m_selectedTask);
        wxString name = wxString::Format(wxT("%X"), id);
        info.SetId(m_selectedTask);
        info.SetColumn(0);
        info.SetMask(wxLIST_MASK_IMAGE);
        m_lstTaskList->GetItem(info);
        if (info.GetImage() == WI_TSK_IDLE || info.GetImage() == WI_TSK_PAUSED) {
            m_client->DoCmd(wxT("runtask"), name);
        }
        else if (info.GetImage() == WI_TSK_RUN) {
            m_client->DoCmd(wxT("pausetask"), name);
        }

        ProcessTaskList();
    }
}

void wiMainForm::OnCancelTask( wxCommandEvent& event )
{
    wxListItem info;

    if (m_selectedTask > -1)
    {
        int id = m_lstTaskList->GetItemData(m_selectedTask);
        wxString name = wxString::Format(wxT("%X"), id);
        m_client->DoCmd(wxT("canceltask"), name);

        ProcessTaskList();
    }
}

void wiMainForm::OnTaskSelected( wxListEvent& event )
{
    SelectTask(event.GetIndex());
}

void wiMainForm::SelectTask(int id/* = -1*/)
{
    wxListItem info;

    if (m_selectedTask != id) {
        // check options changing and ask for save them
        // refresh task options
    }
    m_selectedTask = id;
    if (m_selectedTask > -1) {
        info.SetId(m_selectedTask);
        info.SetColumn(0);
        info.SetMask(wxLIST_MASK_IMAGE);
        m_lstTaskList->GetItem(info);
        if (info.GetImage() == WI_TSK_IDLE) {
            m_bpTaskGo->Enable();
            m_bpTaskGo->SetBitmapLabel( wxBitmap(start_xpm) );
            m_bpCancelTask->Disable();
            m_bpTaskDel->Enable();
            m_panTaskOpts->Enable();
            m_cbInvent->Disable();
        }
        else if (info.GetImage() == WI_TSK_RUN) {
            m_bpTaskGo->Enable();
            m_bpTaskGo->SetBitmapLabel( wxBitmap(pause_xpm) );
            m_bpCancelTask->Enable();
            m_bpTaskDel->Disable();
            m_panTaskOpts->Disable();
        }
        else if (info.GetImage() == WI_TSK_PAUSED) {
            m_bpTaskGo->Enable();
            m_bpTaskGo->SetBitmapLabel( wxBitmap(start_xpm) );
            m_bpCancelTask->Enable();
            m_bpTaskDel->Disable();
            m_panTaskOpts->Disable();
        }
    }
    else {
        m_bpTaskGo->Disable();
        m_bpCancelTask->Disable();
        m_bpTaskDel->Disable();
        //m_panTaskOpts->Hide();
        m_panTaskOpts->Enable();
        m_panTaskOpts->Disable();
    }
    Layout();
}

void wiMainForm::OnSortItems( wxListEvent& event )
{
    sortOrder = ~sortOrder;
    m_lstTaskList->SortItems(SortItemFunc, (long)m_lstTaskList);
}

void wiMainForm::GetPluginList()
{
    int i;
    wxGBSizerItem* item;
    size_t lstSize;

    m_pluginsDock->Freeze();
    for(i = 0; i < m_plugins; i++) {
        item = m_gbPluginsGrid->FindItemAtPosition(wxGBPosition(i+1, 0));
        if (item != NULL) {
            m_gbPluginsGrid->Remove(item->GetWindow());
            //delete item;
        }
        item = m_gbPluginsGrid->FindItemAtPosition(wxGBPosition(i+1, 1));
        if (item != NULL) {
            m_gbPluginsGrid->Remove(item->GetWindow());
            //delete item;
        }
        item = m_gbPluginsGrid->FindItemAtPosition(wxGBPosition(i+1, 2));
        if (item != NULL) {
            m_gbPluginsGrid->Remove(item->GetWindow());
            //delete item;
        }
        item = m_gbPluginsGrid->FindItemAtPosition(wxGBPosition(i+1, 3));
        if (item != NULL) {
            m_gbPluginsGrid->Remove(item->GetWindow());
            //delete item;
        }
    }
    if (m_plugList != NULL) {
        delete m_plugList;
    }
    if(m_client != NULL) {
        m_plugList = m_client->GetPluginList();
        if (m_plugList != NULL) {
            m_plugins = 0;
            for (lstSize = 0; lstSize < m_plugList->size(); lstSize++) {
                wxString label;

                label = wxString::FromAscii((*m_plugList)[lstSize].PluginDesc.c_str());

                wxStaticText *txt = new wxStaticText( m_pluginsDock, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, 0 );
                m_gbPluginsGrid->Add(txt, wxGBPosition(m_plugins+1, 1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);

                wxChoice *cho = new wxChoice( m_pluginsDock, wxID_ANY, wxDefaultPosition, wxSize(120, -1), 0, NULL, 0 );
                for (i = 0; i < (*m_plugList)[lstSize].IfaceList.size(); i++)
                {
                    cho->Append(wxString::FromAscii((*m_plugList)[lstSize].IfaceList[i].c_str()));
                }
                if (cho->GetCount() > 0) {
                    cho->SetSelection(cho->GetCount() - 1);
                }
                m_gbPluginsGrid->Add(cho, wxGBPosition(m_plugins+1, 2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);

                wxButton *btn = new wxButton( m_pluginsDock, wxPluginsData + m_plugins, _("Settings"), wxDefaultPosition, wxDefaultSize);
                btn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wiMainForm::OnPluginSettings ), NULL, this );
                m_gbPluginsGrid->Add(btn, wxGBPosition(m_plugins+1, 3), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);

                char** xpm = StringListToXpm((*m_plugList)[lstSize].PluginIcon);
                wxStaticBitmap *ico = new wxStaticBitmap( m_pluginsDock, wxID_ANY, wxBitmap(xpm));
                m_gbPluginsGrid->Add(ico, wxGBPosition(m_plugins+1, 0), wxDefaultSpan, wxALIGN_CENTER);

                m_plugins++;
            }
            m_gbPluginsGrid->Layout();
        }
    }
    m_pluginsDock->Thaw();
}

void wiMainForm::OnPluginSettings( wxCommandEvent& event )
{
    int id = event.GetId();
    id -= wxPluginsData;

    if (m_client != NULL && (id >= 0 && id < m_plugList->size())) {
        wxString str = wxString::FromAscii((*m_plugList)[id].PluginId.c_str());
        wxString xrc = m_client->DoCmd(wxT("plgui"), str);
        if (xrc.IsEmpty()) {
            wxMessageBox(_("This plugin doesn't provide any settings"), wxT("WebInvent"), wxICON_WARNING | wxOK, this);
        }
        else {
            str += wxT(".xrc");
            wxMemoryFSHandler::AddFile(str, xrc);
            wxString fname = wxT("memory:") + str;
            if ( wxXmlResource::Get()->Load(fname) ) {
                wxDialog dlg;
                if ( wxXmlResource::Get()->LoadDialog(&dlg, this, wxT("SettingsDialog")) ) {
                    dlg.ShowModal();
                    /// @todo Get the controls values, pack them to the XML and sends back
                    // m_client->DoCmd("applysettings", xmlData);
                }
                wxXmlResource::Get()->Unload(fname);
            }
            else {
                wxMessageBox(_("Can't load given UI"), wxT("WebInvent"), wxICON_STOP | wxOK, this);
            }
            wxMemoryFSHandler::RemoveFile(str);
        }
    }
}

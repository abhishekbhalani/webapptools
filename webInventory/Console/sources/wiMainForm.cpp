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
#include <wx/mstream.h>
#include "wiStatBar.h"
#include "wiServDialog.h"
#include "wiObjDialog.h"
#include "wiMainForm.h"
#include "wxThings.h"
#include "treeData.h"
#include "version.h"
#include "wiReportData.h"
#include "weRttiOptions.h"
#include "wiPlgSelector.h"
#include "../images/webInventory.xpm"
#include "../images/btnStop.xpm"
#include "../images/start.xpm"
#include "../images/pause.xpm"
// treebook
#include "../images/opts_global.xpm"
#include "../images/opts_transport.xpm"
#include "../images/opts_inventory.xpm"
#include "../images/opts_audit.xpm"
#include "../images/opts_vulner.xpm"

#include "optionnames.h"

/// @todo change this
#include "../images/tree_no.xpm"
#include "../images/tree_unk.xpm"
#include "../images/tree_yes.xpm"

#define wxPING_TIMER    999
#define wxPING_INTERVAL 10000

//    m_statusBar = new wiStatBar( this );
//    SetStatusBar(m_statusBar);

DEFINE_EVENT_TYPE(wxEVT_REPORT_LOADING)
DEFINE_EVENT_TYPE(wxEVT_REPORT_LOAD)

static const wxChar* gTaskStatus[] = {_("idle"),
                                      _("run (%d%%)"),
                                      _("paused (%d%%)")};

wiSimpleReport repWait(_("Please wait..."));
wiSimpleReport repPlaceHold(_("Report will be here"));
wiSimpleReport repError(_("Can't retreiving information for task!"), 1);
wiSimpleReport repNoConnet(_("No connection to server!"), 1);

wxString FromStdString(const std::string& str)
{
    wxString retval = wxString::FromUTF8(str.c_str());
    return retval;
}

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

wxPanel* CreateDefaultPanel(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);

    (void) new wxStaticText( panel, wxID_ANY, _("This plugin doesn't provide any settings"), wxDefaultPosition, wxDefaultSize, 0 );

    return panel;
}

wxTreeItemId FindItem(wxTreeCtrl* tree, wxTreeItemId curr, const wxString& label)
{
    if (tree->GetItemText(curr) == label) {
        return curr;
    }
    wxTreeItemId child, search;
    wxTreeItemIdValue cookie;

    child = tree->GetFirstChild(curr, cookie);
    while (child.IsOk()) {
        search = FindItem(tree, child, label);
        if (search.IsOk()) {
            break;
        }
        child = tree->GetNextChild(curr, cookie);
    }

    return search;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//                   wiMainForm class functions
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
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

    iData = 0;
    if (m_cfgEngine.Read(wxT("logging"), &iData)) {
        m_chLogging->SetValue(iData);
    }

    m_lstImages.Add(wxIcon(tree_unk));
    m_lstImages.Add(wxIcon(tree_yes));
    m_lstImages.Add(wxIcon(tree_no));

    m_lstTaskList->InsertColumn(0, wxT(""), wxLIST_FORMAT_LEFT, 20);
    m_lstTaskList->InsertColumn(1, _("Task name"), wxLIST_FORMAT_LEFT, 180);
    m_lstTaskList->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, 96);
    m_lstTaskList->SetImageList(&m_lstImages, wxIMAGE_LIST_SMALL);

    m_lstTaskList->DeleteAllItems();

    m_lstObjectList->InsertColumn(0, wxT("Object name"), wxLIST_FORMAT_LEFT, 180);
    m_lstObjectList->DeleteAllItems();

    m_plugins = 0;
    m_plugList = NULL;

    m_bpAddPlugin->Enable(false);
    m_bpDelPlugin->Enable(false);

    m_toolBarObject->EnableTool(wxID_TOOLGO, false);
    m_toolBarObject->EnableTool(wxID_TOOLSTOP, false);
    m_toolBarObject->EnableTool(wxID_TOOLNEW, false);
    m_toolBarObject->EnableTool(wxID_TOOLDEL, false);

    // create default view of the options panel
    RebuildOptionsView();

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
    m_selectedProf = -1;
    m_pnServer->Hide();
    m_panTaskOpts->Disable();
    m_lstTaskList->Disable();
    m_toolBarObject->Disable();
    m_pReports->Disable();
    m_mainnb->SetSelection(2);
    Layout();

    this->Connect( wxID_ANY, wxEVT_REPORT_LOADING, wxCommandEventHandler( wiMainForm::OnReportsLoadStart ) );
    this->Connect( wxID_ANY, wxEVT_REPORT_LOAD, wxCommandEventHandler( wiMainForm::OnReportsLoad ) );
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
            ProcessObjects(wxT(""));
            ProcessProfileList(wxT(""));
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
    m_cfgEngine.Write(wxT("logging"), m_chLogging->GetValue());

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

            m_client = new wiTcpClient(host.ToAscii(), port.ToAscii());
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
    RebuildReportsTree();
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

void wiMainForm::ProcessObjects(const wxString& criteria /*= wxT("")*/)
{
    ObjectList* lst;
    size_t lstSize;
    int idx = 0;

    if(m_client != NULL) {
        lst = m_client->GetObjectList(criteria);
        if (lst != NULL) {
            wxWindowUpdateLocker taskList(m_lstObjectList);

            m_lstObjectList->DeleteAllItems();
            for (lstSize = 0; lstSize < lst->size(); lstSize++) {
                wxString idStr = FromStdString((*lst)[lstSize].Name.c_str());
                idx = m_lstObjectList->GetItemCount();
                m_lstObjectList->InsertItem(idx, idStr);
                m_lstObjectList->SetItemData(idx, (wxUIntPtr)(&((*lst)[lstSize])));
            }

            FillObjectFilter();
        }
        else {
            m_statusBar->SetImage(wiSTATUS_BAR_NO);
            m_statusBar->SetStatusText(m_client->GetLastError(), 3);
        }
    }
}

void wiMainForm::ProcessTaskList(const wxString& criteria /*= wxT("")*/)
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
                wxString idStr = FromStdString((*lst)[lstSize].id.c_str());
                idStr.ToLong(&idLong, 16);
                idx = m_lstTaskList->GetItemCount();
                m_lstTaskList->InsertItem(idx, (*lst)[lstSize].status);
                m_lstTaskList->SetItem(idx, 1, FromStdString((*lst)[lstSize].name));
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
    m_toolBarObject->EnableTool(wxID_TOOLNEW, false);
    m_pReports->Disable();
    m_pnServer->Disable();
    m_panTaskOpts->Disable();
    m_lstTaskList->Disable();
    m_toolBarObject->Disable();
    m_statusBar->SetImage(wiSTATUS_BAR_NO);
    m_statusBar->SetStatusText(_("Disconnected"), 1);
    m_statusBar->SetStatusText(_("unknown"), 2);
    m_statusBar->SetStatusText(m_client->GetLastError(), 3);
    if (mode) {
        m_toolBarSrv->SetToolShortHelp(wxID_TLCONNECT, _("Connect"));
        m_toolBarSrv->SetToolNormalBitmap(wxID_TLCONNECT, wxBitmap(start_xpm));
//        m_bpConnect->SetToolTip(_("Connect"));
//        m_bpConnect->SetBitmapLabel(wxBitmap(start_xpm));
        m_pnServer->Hide();
        Layout();
    }
}

void wiMainForm::Connected(bool mode)
{
    m_toolBarObject->EnableTool(wxID_TOOLNEW, true);
    m_pReports->Enable();
    m_pnServer->Enable();
    m_panTaskOpts->Enable();
    m_lstTaskList->Enable();
    m_toolBarObject->Enable();
    m_statusBar->SetImage(wiSTATUS_BAR_YES);
    m_statusBar->SetStatusText(_("Connected"), 1);
    m_statusBar->SetStatusText(wxT(""), 3);
    if (mode) {
        m_toolBarSrv->SetToolShortHelp(wxID_TLCONNECT, _("Disconnect"));
        m_toolBarSrv->SetToolNormalBitmap(wxID_TLCONNECT, wxBitmap(btnStop_xpm));
//        m_bpConnect->SetToolTip(_("Disconnect"));
//        m_bpConnect->SetBitmapLabel(wxBitmap(btnStop_xpm));
        ProcessTaskList();
        ProcessObjects();
        ProcessProfileList();
        m_pnServer->Show();
        Layout();
    }
}

void wiMainForm::OnSelectObject( wxListEvent& event )
{
    m_selectedObject = event.GetIndex();
}

void wiMainForm::OnAddObject( wxCommandEvent& event )
{
    wiObjDialog objDlg(this);
    wxString name, url;
    ObjectInfo objInfo;

    if (objDlg.ShowModal() == wxOK) {
        // save new object
        objInfo.ObjectId = "0";
        objInfo.Name = objDlg.m_txtObjName->GetValue().utf8_str();
        objInfo.Address = objDlg.m_txtBaseURL->GetValue().utf8_str();
        if (m_client != NULL) {
            m_client->UpdateObject(objInfo);
            ProcessObjects();
        }
    }
}

void wiMainForm::OnEditObject( wxCommandEvent& event )
{
    wiObjDialog objDlg(this);
    wxString name, url;
    ObjectInfo* obj;

    if (m_selectedObject > -1) {
        obj = (ObjectInfo*)m_lstObjectList->GetItemData(m_selectedObject);
        objDlg.m_txtObjName->SetValue(FromStdString(obj->Name));
        objDlg.m_txtBaseURL->SetValue(FromStdString(obj->Address));
        if (objDlg.ShowModal() == wxOK) {
            // save new object
            obj->Name = objDlg.m_txtObjName->GetValue().utf8_str();
            obj->Address = objDlg.m_txtBaseURL->GetValue().utf8_str();
            if (m_client != NULL) {
                m_client->UpdateObject(*obj);
                //ProcessObjects();
            }
        }
    }
}

void wiMainForm::OnDelObject( wxCommandEvent& event )
{
    wxString name;
    wxListItem info;
    ObjectInfo* obj;

    if (m_selectedObject > -1)
    {
        info.SetId(m_selectedObject);
        info.SetColumn(0);
        info.SetMask(wxLIST_MASK_TEXT);
        m_lstObjectList->GetItem(info);
        name = wxString::Format(_("Are you sure to delete object '%s'?"), info.GetText().c_str());
        int res = wxMessageBox(name, _("Confirm"), wxYES_NO | wxICON_QUESTION, this);
        if (res == wxYES) {
            obj = (ObjectInfo*)m_lstObjectList->GetItemData(m_selectedObject);
            m_client->DoCmd(wxT("delobj"), FromStdString(obj->ObjectId));
            //ProcessObjects();
        }
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
        name = wxString::Format(_("Are you sure to delete task '%s'?"), info.GetText().c_str());
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
    int prevID = m_selectedTask;

    if (m_selectedTask != id && m_selectedTask > -1) {
        info.SetId(m_selectedTask);
        info.SetColumn(0);
        info.SetMask(wxLIST_MASK_DATA);
        m_lstTaskList->GetItem(info);
        // check options changing and ask for save them
    }
    m_selectedTask = id;
    if (m_selectedTask > -1) {
        info.SetId(m_selectedTask);
        info.SetColumn(0);
        info.SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_DATA);
        m_lstTaskList->GetItem(info);
        if (info.GetImage() == WI_TSK_IDLE) {
            //m_btnApply->Enable();
            m_toolBarTasks->EnableTool(wxID_TOOLPAUSE, true);
            m_toolBarTasks->SetToolNormalBitmap(wxID_TOOLPAUSE, wxBitmap(start_xpm) );
            m_toolBarTasks->EnableTool(wxID_TOOLSTOP, false);
        }
        else if (info.GetImage() == WI_TSK_RUN) {
            //m_btnApply->Disable();
            m_toolBarTasks->EnableTool(wxID_TOOLPAUSE, true);
            m_toolBarTasks->SetToolNormalBitmap(wxID_TOOLPAUSE, wxBitmap(pause_xpm) );
            m_toolBarTasks->EnableTool(wxID_TOOLSTOP, true);
        }
        else if (info.GetImage() == WI_TSK_PAUSED) {
            //m_btnApply->Disable();
            m_toolBarTasks->EnableTool(wxID_TOOLPAUSE, true);
            m_toolBarTasks->SetToolNormalBitmap(wxID_TOOLPAUSE, wxBitmap(start_xpm) );
            m_toolBarTasks->EnableTool(wxID_TOOLSTOP, true);
        }
        /// @todo refresh task log
    }
    else {
        //m_btnApply->Enable();
        m_toolBarTasks->EnableTool(wxID_TOOLPAUSE, false);
        m_toolBarTasks->EnableTool(wxID_TOOLSTOP, false);
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
    m_chStorage->Freeze();
    m_chStorage->Clear();
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
    }
    if (m_plugList != NULL) {
        delete m_plugList;
    }
    if(m_client != NULL) {
        wxString storageID = m_client->DoCmd(wxT("getstorage"), wxT(""));
        m_plugList = m_client->GetPluginList();
        if (m_plugList != NULL) {
            m_plugins = 0;
            for (lstSize = 0; lstSize < m_plugList->size(); lstSize++) {
                wxString label;

                label = FromStdString((*m_plugList)[lstSize].PluginDesc);

                wxStaticText *txt = new wxStaticText( m_pluginsDock, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, 0 );
                m_gbPluginsGrid->Add(txt, wxGBPosition(m_plugins+1, 1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);

                wxChoice *cho = new wxChoice( m_pluginsDock, wxID_ANY, wxDefaultPosition, wxSize(120, -1), 0, NULL, 0 );
                for (i = 0; i < (*m_plugList)[lstSize].IfaceList.size(); i++)
                {
                    cho->Append(FromStdString((*m_plugList)[lstSize].IfaceList[i]));
                }
                if (cho->GetCount() > 0) {
                    cho->SetSelection(cho->GetCount() - 1);
                }
                m_gbPluginsGrid->Add(cho, wxGBPosition(m_plugins+1, 2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);

                char** xpm = StringListToXpm((*m_plugList)[lstSize].PluginIcon);
                wxStaticBitmap *ico = new wxStaticBitmap( m_pluginsDock, wxID_ANY, wxBitmap(xpm));
                m_gbPluginsGrid->Add(ico, wxGBPosition(m_plugins+1, 0), wxDefaultSpan, wxALIGN_CENTER);

                // add plugins to the storage list
                if (find((*m_plugList)[lstSize].IfaceList.begin(), (*m_plugList)[lstSize].IfaceList.end(), "iweStorage") != (*m_plugList)[lstSize].IfaceList.end()) {
                    int itm = m_chStorage->Append(FromStdString((*m_plugList)[lstSize].PluginDesc), (void*)(wxPluginsData + m_plugins));
                    if (storageID.CmpNoCase(FromStdString((*m_plugList)[lstSize].PluginId)) == 0) {
                        m_chStorage->SetSelection(itm);
                    }
                }

                m_plugins++;
            }
            m_gbPluginsGrid->Layout();
        }
    }
    m_chStorage->Thaw();
    m_pluginsDock->Thaw();
}

wxPanel* wiMainForm::LoadPluginSettings( PluginInfo* plg )
{
    wxString id = FromStdString(plg->PluginId);
    wxPanel* panel = NULL;

    if (m_client != NULL) {
        wxString xrc = m_client->DoCmd(wxT("plgui"), id);
        if (!xrc.IsEmpty()) {
            id += wxT(".xrc");
            wxMemoryFSHandler::AddFile(id, xrc);
            wxString fname = wxT("memory:") + id;
            if ( wxXmlResource::Get()->Load(fname) ) {
                panel = wxXmlResource::Get()->LoadPanel(m_plgBook, wxT("ui_panel"));
                wxXmlResource::Get()->Unload(fname);
            }
            else {
                wxMessageBox(_("Can't load given UI"), wxT("WebInvent"), wxICON_STOP | wxOK, this);
            }
            wxMemoryFSHandler::RemoveFile(id);
        }
    }

    if (panel == NULL) {
        panel = CreateDefaultPanel(m_plgBook);
    }
    char** xpm = StringListToXpm((vector<string>)plg->PluginIcon);
    int imgIdx = m_plgBook->GetImageList()->Add(wxBitmap(xpm));
    wxString pageLabel;
    int pageIdx = 0;

    if (find(plg->IfaceList.begin(), plg->IfaceList.end(), "iweTransport") != plg->IfaceList.end())
    {   // add plugins to the transport list
        pageLabel = _("Transports");
    }
    else if (find(plg->IfaceList.begin(), plg->IfaceList.end(), "iweInventory") != plg->IfaceList.end())
    {   // add plugins to the inventory list
        pageLabel = _("Inventory");
    }
    else if (find(plg->IfaceList.begin(), plg->IfaceList.end(), "iweAudit") != plg->IfaceList.end())
    {   // add plugins to the audit list
        pageLabel = _("Audit");
    }
    else if (find(plg->IfaceList.begin(), plg->IfaceList.end(), "iweVulner") != plg->IfaceList.end())
    {   // add plugins to the vulnerabilities list
        pageLabel = _("Vulnerabilities");
    }
    else
    {   // add plugins to the generic list
        pageLabel = _("Generic options");
    }
    for (int i = 0; i < m_plgBook->GetPageCount(); i++) {
        if (m_plgBook->GetPageText(i) == pageLabel) {
            pageIdx = i;
            break;
        }
    }
    pageLabel = FromStdString(plg->IfaceList.back());
    panel->SetName(id);
    m_plgBook->InsertSubPage(pageIdx, panel, pageLabel, false, imgIdx);

    return panel;
}

void wiMainForm::GetTaskOptions(const wxString& taskID)
{
    wxString optsstr;

    if (m_client != NULL) {
        optsstr = m_client->DoCmd(wxT("getprofileopts"), taskID);
        //optsstr = wxString(wxT("<?xml version=\"1.0\"?>")) + optsstr;
        wxCharBuffer buff = optsstr.utf8_str();
        int buffLen = strlen(buff.data());
        wxMemoryInputStream xmlStream(buff.data(), buffLen);
        wxXmlDocument opt;
        if(opt.Load(xmlStream)) {
            wxXmlNode *root = opt.GetRoot();
            if (root && root->GetName().CmpNoCase(wxT("report")) == 0) {
                wxXmlNode *child = root->GetChildren();
                //search for 'profile'
                while (child != NULL) {
                    if (child->GetName().CmpNoCase(wxT("profile")) == 0) {
                        break;
                    }
                }
                if (child == NULL) {
                    return;
                }
                child = child->GetChildren();
                //search for 'options'
                while (child != NULL) {
                    if (child->GetName().CmpNoCase(wxT("options")) == 0) {
                        break;
                    }
                }
                if (child == NULL) {
                    return;
                }
                root = child;
                child = root->GetChildren();

                // reset valuses
                m_txtProfName->SetValue(wxT(""));
                m_chLogLevel->Select(0);
                RebuildOptionsView();
                /// @todo reset values to default
//                m_txtBaseURL->SetValue(wxT(""));
//                m_rbInDir->SetValue(false);
//                m_rbInHost->SetValue(false);
//                m_rbInDomain->SetValue(false);
//                m_txtDepth->SetValue(wxT("-1"));

                // set new values
                while (child != NULL) {
                    if (child->GetType() == wxXML_ELEMENT_NODE && child->GetName().CmpNoCase(wxT("option")) == 0) {
                        wxString name = child->GetPropVal(wxT("name"), wxT(""));
                        wxString type = child->GetPropVal(wxT("type"), wxT(""));
                        wxString text = child->GetNodeContent();
                        // fill the data controls
                        if (name.CmpNoCase(wxT(weoName)) == 0) {
                            m_txtProfName->SetValue(text);
                        }
                        if (name.CmpNoCase(wxT("plugin_list")) == 0) {
                            // parse string and create options panels
                            int pos = text.Find(wxT(';'));
                            while (pos != wxNOT_FOUND) {
                                wxString pid = text.Mid(0, pos);
                                PluginInfo *plg = FoundPlugin(pid);
                                if (plg != NULL) {
                                    LoadPluginSettings(plg);
                                }
                                text = text.Mid(pos + 1);
                                pos = text.Find(wxT(';'));
                            }
                            PluginInfo *plg = FoundPlugin(text);
                            if (plg != NULL) {
                                LoadPluginSettings(plg);
                            }
                        }

//                        if (name.CmpNoCase(wxT(weoBaseURL)) == 0) {
//                            //m_txtBaseURL->SetValue(text);
//                        }
//                        if (name.CmpNoCase(wxT(weoStayInDir)) == 0 && text == wxT("1")) {
//                            m_rbInDir->SetValue(true);
//                        }
//                        if (name.CmpNoCase(wxT(weoStayInHost)) == 0 && text == wxT("1")) {
//                            m_rbInHost->SetValue(true);
//                        }
//                        if (name.CmpNoCase(wxT(weoStayInDomain)) == 0 && text == wxT("1")) {
//                            m_rbInDomain->SetValue(true);
//                        }
//                        if (name.CmpNoCase(wxT(weoScanDepth)) == 0) {
//                            m_txtDepth->SetValue(text);
//                        }
//                        if (name.CmpNoCase(wxT(weoLogLevel)) == 0) {
//                            long idx;
//                            text.ToLong(&idx);
//                            if (idx >= 0 && idx < m_chLogLevel->GetCount()) {
//                                m_chLogLevel->Select(idx);
//                            }
//                        }
                    }

                    child = child->GetNext();
                } // end of first iteration
                // fill panel's data
                for (int i = 1; i < m_plgBook->GetPageCount(); i++) {
                    weRttiOptions::SetOptions(m_plgBook->GetPage(i), root);
                }
            } // end XML processing
        }
    }
}

void wiMainForm::OnStorageChange( wxCommandEvent& event )
{
    int plg = m_chStorage->GetSelection();
    if (plg != -1 && m_plugList != NULL && m_client != NULL) {
        int plgIdx = (int)m_chStorage->GetClientData(plg) - wxPluginsData;
        if (plgIdx >= 0 && plgIdx < m_plugList->size()) {
            wxString plgID = FromStdString((*m_plugList)[plgIdx].PluginId);
            m_client->DoCmd(wxT("setstorage"), plgID);
            ProcessTaskList(wxT(""));
        }
    }
}

void wiMainForm::FillObjectFilter()
{
    int tasks, i;
    wxListItem info;
    long taskID;
    int taskIndex, tskSelected, tskID;
    wxString idStr;
    ObjectInfo* obj;

    m_chTaskFilter->Clear();
    if (! m_cfgEngine.Read(wxT("ObjectFilter"), &tskID)) {
        tskID = -1;
    }
    tskSelected = -1;
    taskIndex = m_chTaskFilter->Append(_("All"), (void*)-1);
    if (-1 == tskID) { // read selected from profile
        tskSelected = taskIndex;
    }
    tasks = m_lstObjectList->GetItemCount();
    for (i = 0; i < tasks; i++) {
        obj = (ObjectInfo*)m_lstObjectList->GetItemData(i);
        idStr = FromStdString(obj->ObjectId);
        idStr.ToLong(&taskID, 16);
        idStr = FromStdString(obj->Name);
        taskIndex = m_chTaskFilter->Append(idStr, (void*)taskID);
        if (taskID == tskID) { // read selected from profile
            tskSelected = taskIndex;
        }
    }
    if (tskSelected > -1) {
        m_chTaskFilter->Select(tskSelected);
    }
}

void wiMainForm::OnReportTskFilter( wxCommandEvent& event )
{
    int tskIndex = m_chTaskFilter->GetSelection();
    int tskFilter = (int)m_chTaskFilter->GetClientData(tskIndex);
    m_cfgEngine.Write(wxT("TaskFilter"), tskFilter);
    m_cfgEngine.Flush();
}

void wiMainForm::OnReportsFilter( wxCommandEvent& event )
{
    RebuildReportsTree();
}

void wiMainForm::OnReportsRefresh( wxCommandEvent& event )
{
    RebuildReportsTree();
}

void wiMainForm::RebuildReportsTree()
{
    bool isFilter = m_toolBarFilter->GetToolState(wxID_TLFILTER);
    int tasks, i;
    wxListItem info;
    long taskID;
    int taskIndex, tskID;
    wxString idStr;
    wxTreeItemId root;
    wxTreeItemId task;
    ObjectInfo* obj;

    m_treeScans->Freeze();
    m_treeScans->DeleteAllItems();
    taskIndex = m_chTaskFilter->GetSelection();
    if (taskIndex < 0) {
        return;
    }
    tskID = (int)m_chTaskFilter->GetClientData(taskIndex);
    tasks = m_lstObjectList->GetItemCount();
    root = m_treeScans->AddRoot(wxT("root"));
    for (i = 0; i < tasks; i++) {
        obj = (ObjectInfo*)m_lstObjectList->GetItemData(i);
        idStr = FromStdString(obj->ObjectId);
        idStr.ToLong(&taskID, 16);
        if ( !isFilter || taskID == tskID || tskID == -1) {
            idStr = FromStdString(obj->Name);
            wiTreeData *data = new wiTreeData;
            data->nodeType = WI_TREE_NODE_OBJECT;
            data->objectID = taskID;
            task = m_treeScans->AppendItem(root, idStr, -1, -1, data);
            m_treeScans->AppendItem(task, _("Please wait..."));
        }
    }

    m_treeScans->Thaw();
}

void wiMainForm::OnReportExpand( wxTreeEvent& event )
{
    wxTreeItemId object = event.GetItem();
    wiTreeData *data = (wiTreeData*)m_treeScans->GetItemData(object);

    if (data != NULL) {
        if (!data->hasData) {
            repWait.WriteReport(*m_richText2);
            wxCommandEvent event( wxEVT_REPORT_LOADING, wxID_ANY );
            event.SetClientData((void*)data);
            GetEventHandler()->AddPendingEvent( event );
        }
        else {
            data->hasData->WriteReport(*m_richText2);
        }
    }
}

void wiMainForm::OnReportSelected( wxTreeEvent& event )
{
    wxTreeItemId object = event.GetItem();
    wiTreeData *data = (wiTreeData*)m_treeScans->GetItemData(object);

    if (data != NULL) {
        if (!data->hasData) {
            repWait.WriteReport(*m_richText2);
            wxCommandEvent event( wxEVT_REPORT_LOADING, wxID_ANY );
            event.SetClientData((void*)data);
            GetEventHandler()->AddPendingEvent( event );
        }
        else {
            data->hasData->WriteReport(*m_richText2);
        }
    }
}

void wiMainForm::OnReportsLoadStart( wxCommandEvent& event )
{   // just resend event after processing GUI events
    wxCommandEvent event2( wxEVT_REPORT_LOAD, wxID_ANY );
    event2.SetClientData(event.GetClientData());
    wxSafeYield(this);
    ::wxBeginBusyCursor();
    GetEventHandler()->AddPendingEvent( event2 );
}

void wiMainForm::OnReportsLoad( wxCommandEvent& event )
{
    int i, n;
    long tskId, scanId;
    wxTreeItemId scan;
    wiTreeData *scanData;

    wiTreeData *data = (wiTreeData*)event.GetClientData();
    if (data != NULL) {
        if (data->nodeType == WI_TREE_NODE_OBJECT) {
            // builds list of scans for this object (task for now)
            m_treeScans->DeleteChildren(data->GetId());
            if (m_client != NULL) {
                ScanList* lst = m_client->GetScanList(wxString::Format(wxT("ID=%X"), data->objectID));
                if (lst != NULL) {
                    n = 0;
                    for (i = 0; i < lst->size(); i++) {
                        wxString idStr = FromStdString((*lst)[i].ObjectId);
                        idStr.ToLong(&tskId, 16);
                        if (tskId == data->objectID) {
                            idStr = FromStdString((*lst)[i].ScanId);
                            idStr.ToLong(&scanId, 16);
                            idStr = FromStdString((*lst)[i].StartTime);
                            /// @todo Apply date filter
                            scanData = new wiTreeData;
                            scanData->nodeType = WI_TREE_NODE_SCAN;
                            scanData->objectID = scanId;
                            scan = m_treeScans->AppendItem(data->GetId(), idStr, -1, -1, scanData);
                            m_treeScans->AppendItem(scan, _("Please wait..."));
                            n++;
                        }
                    }
                    // write summary for scan
                    wiSimpleReport* rep = new wiSimpleReport(wxString::Format(wxT("Report for task: %s\nScans: %d"), wxT("***"), n));

                    // save report
                    rep->WriteReport(*m_richText2);
                    data->hasData = rep;

                    delete lst;
                }
                else {
                    // generate error report
                    repError.WriteReport(*m_richText2);
                }
            }
            else {
                // generate error report
                repNoConnet.WriteReport(*m_richText2);
            }
        }
        else {
            wxSleep(3);
            wiSimpleReport* rep = new wiSimpleReport(_("Report will be here"));
            rep->WriteReport(*m_richText2);
            data->hasData = rep;
        }
    }
    ::wxEndBusyCursor();
}

void wiMainForm::ProcessProfileList(const wxString& criteria /*= wxT("")*/)
{
    ProfileList* lst;
    ProfileInfo* dat;
    size_t lstSize;
    int idx = 0;
    int selected;
    wxString idSelect = wxT("0");

    selected = m_chProfile->GetSelection();
    if (selected > -1) {
        dat = (ProfileInfo*)m_chProfile->GetClientData(selected);
        idSelect = FromStdString(dat->ObjectId);
    }
    if(m_client != NULL) {
        lst = m_client->GetProfileList(criteria);
        if (lst != NULL) {
            wxWindowUpdateLocker taskList(m_chProfile);

            m_chProfile->Clear();
            selected = -1;
            for (lstSize = 0; lstSize < lst->size(); lstSize++) {
                wxString idStr = FromStdString((*lst)[lstSize].Name);
                idx = m_chProfile->GetCount();
                idx = m_chProfile->Insert(idStr, idx, (void*)(&((*lst)[lstSize])));
                idStr = FromStdString((*lst)[lstSize].ObjectId);
                if (idStr == idSelect) {
                    selected = idx;
                }
            }
            if (selected > -1) {
                // restore selection
                m_chProfile->Select(selected);
            }
        }
        else {
            m_statusBar->SetImage(wiSTATUS_BAR_NO);
            m_statusBar->SetStatusText(m_client->GetLastError(), 3);
        }
    }

}

void wiMainForm::OnAddProfile( wxCommandEvent& event )
{
    wxString name;

    name = wxGetTextFromUser(_("Input new profile name"), _("Query"), wxT(""), this);
    if (!name.IsEmpty() && m_client != NULL) {
        name = m_client->DoCmd(wxT("addprofile"), name);
    }
    ProcessProfileList();
}

void wiMainForm::OnCopyProfile( wxCommandEvent& event )
{
    ProfileInfo* dat;
    int selected;
    wxString idSelect;
    wxString idPrev;
    wxString xml;

    selected = m_chProfile->GetSelection();
    if (selected > -1) {
        dat = (ProfileInfo*)m_chProfile->GetClientData(selected);
        idSelect = FromStdString(dat->Name) + wxT(" copy");
        if(m_client != NULL) {
            idSelect = m_client->DoCmd(wxT("addprofile"), idSelect);
            if (idSelect != wxT("") && idSelect != wxT("0")) {
                idPrev = FromStdString(dat->ObjectId);
                xml = m_client->DoCmd(wxT("getprofileopts"), idPrev);
                /// @todo change ID and Name
                idPrev = wxString::Format(wxT("%s;%s"), idSelect.c_str(), xml.c_str());
                m_client->DoCmd(wxT("setprofileopts"), idPrev);

                ProcessProfileList();
            }
        }
    }
}

void wiMainForm::OnDelProfile( wxCommandEvent& event )
{
    wxString name;
    ProfileInfo* dat;
    int selected;

    selected = m_chProfile->GetSelection();
    if (selected > -1)
    {
        name = m_chProfile->GetString(selected);
        dat = (ProfileInfo*)m_chProfile->GetClientData(selected);
        name = wxString::Format(_("Are you sure to delete profile '%s'?"), name.c_str());
        int res = wxMessageBox(name, _("Confirm"), wxYES_NO | wxICON_QUESTION, this);
        if (res == wxYES) {
            name = FromStdString(dat->ObjectId);
            m_client->DoCmd(wxT("delprofile"), name);

            ProcessProfileList();
        }
    }
}

void wiMainForm::OnTaskApply( wxCommandEvent& event )
{
    ProfileInfo* dat;
    wxString profID;

    // save profile data
    if (m_selectedProf > -1) {
        dat = (ProfileInfo*)m_chProfile->GetClientData(m_selectedProf);
        profID.Empty();
        if (dat != NULL) {
            profID = FromStdString(dat->ObjectId);
        }
        // check options changing and ask for save them
        if (m_client != NULL && !profID.IsEmpty()) {
            wxXmlDocument opt;
            wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE, wxT("options"));
            wxString content;

            for (int i = 1; i < m_plgBook->GetPageCount(); i++) {
                int imgIdx = m_plgBook->GetPageImage(i);
                if (imgIdx > 4) {
                    if (!content.IsEmpty()) {
                        content += wxT(";");
                    }
                    content += m_plgBook->GetPage(i)->GetName();
                }
                //weRttiOptions::GetOptions(m_plgBook->GetPage(i), root);
            }
            weRttiOptions::SaveTaskOption (root, wxT("plugin_list"), wxT("8"), content);
            weRttiOptions::GetOptions(m_panTaskOpts, root);

            opt.SetRoot(root);
            wxMemoryOutputStream outp;
            opt.Save(outp, wxXML_NO_INDENTATION);
            int dataLen = outp.GetSize();
            char *data = new char[dataLen + 10];
            if (data != NULL) {
                outp.CopyTo(data, dataLen);
                data[dataLen] = '\0';
                wxString str = wxString::Format(wxT("%s;"), profID);
                str += wxString::FromUTF8(data);
                m_client->DoCmd(wxT("setprofileopts"), str);
            }
            else {
                wxLogError(wxT("Can't compose options save request!"));
            }
        }
    }
}

void wiMainForm::OnChangeProfile( wxCommandEvent& event )
{
    wxListItem info;
    int prevID = m_selectedProf;
    ProfileInfo* dat;

    m_selectedProf = m_chProfile->GetSelection();

    if (m_selectedProf != prevID && m_selectedTask > -1) {
        // check options changing and ask for save them
    }
    if (m_selectedProf > -1) {
        m_bpAddPlugin->Enable(true);
        m_bpDelPlugin->Enable(true);
        dat = (ProfileInfo*)m_chProfile->GetClientData(m_selectedProf);
        GetTaskOptions(FromStdString(dat->ObjectId));
    }
    else {
        m_bpAddPlugin->Enable(false);
        m_bpDelPlugin->Enable(false);
    }
}

void wiMainForm::OnPlgRefresh( wxCommandEvent& event )
{
    GetPluginList();
}

void wiMainForm::RebuildOptionsView()
{
    wxImageList* icons;
    wxPanel *panel;

    wxWindowUpdateLocker taskList(m_plgBook);

    m_plgBook->GetTreeCtrl()->SetWindowStyle(wxTR_FULL_ROW_HIGHLIGHT|wxTR_HAS_BUTTONS|wxTR_HIDE_ROOT|wxTR_LINES_AT_ROOT|wxTR_SINGLE);

    icons = new wxImageList(16, 16);
    icons->RemoveAll();
    icons->Add(wxBitmap(opts_global_xpm));
    icons->Add(wxBitmap(opts_transport_xpm));
    icons->Add(wxBitmap(opts_inventory_xpm));
    icons->Add(wxBitmap(opts_audit_xpm));
    icons->Add(wxBitmap(opts_vulner_xpm));
    m_plgBook->AssignImageList(icons);

    while (m_plgBook->GetPageCount() > 1) {
        m_plgBook->DeletePage(1);
    }
    m_plgBook->SetPageImage(0, 0);
    // transports
    panel = CreateDefaultPanel(m_plgBook);
    m_plgBook->AddPage(panel, _("Transports"), false, 1);
    // inventory
    panel = CreateDefaultPanel(m_plgBook);
    m_plgBook->AddPage(panel, _("Inventory"), false, 2);
    // audit
    panel = CreateDefaultPanel(m_plgBook);
    m_plgBook->AddPage(panel, _("Audit"), false, 3);
    // vulners
    panel = CreateDefaultPanel(m_plgBook);
    m_plgBook->AddPage(panel, _("Vulnerabilities"), false, 4);

    m_plgBook->SetSelection(0);
}

void wiMainForm::OnOptionsPageChanging( wxTreebookEvent& event )
{
    int idx = event.GetSelection();
    idx = m_plgBook->GetPageImage(idx);
    if (idx >= 1 && idx <= 4) {
        event.Veto();
    }
    else {
        // do something for page changing
    }
}

void wiMainForm::OnAddPlugin( wxCommandEvent& event )
{
    size_t lstSize, i;
    wxTreeItemId parent, current;
    wiPlgSelector dlg(this);

    if (m_plugList == NULL) {
        GetPluginList();
    }
    dlg.m_plgTree->DeleteAllItems();
    if (m_plugList != NULL) {
        dlg.m_plgTree->AddRoot(wxT("iwePlugin"));
        for (lstSize = 0; lstSize < m_plugList->size(); lstSize++) {
            parent = dlg.m_plgTree->GetRootItem();
            for (i = 0; i < (*m_plugList)[lstSize].IfaceList.size(); i++)
            {
                wxString label = FromStdString((*m_plugList)[lstSize].IfaceList[i]);
                current = ::FindItem(dlg.m_plgTree, parent, label);

                if (current.IsOk()) {
                    parent = current;
                    if ((i + 1) == (*m_plugList)[lstSize].IfaceList.size()) {
                        wiPlgTreeData* data = new wiPlgTreeData(&((*m_plugList)[lstSize]));
                        dlg.m_plgTree->SetItemData(current, data);
                    }
                }
                else {
                    wiPlgTreeData* data = new wiPlgTreeData(&((*m_plugList)[lstSize]));
                    parent = dlg.m_plgTree->AppendItem(parent, label, -1, -1, data);
                }
            }
        }
        dlg.m_plgTree->ExpandAll();
        if (dlg.ShowModal() == wxOK) {
            wxString label = dlg.m_plgTree->GetItemText(dlg.m_selected);
            current = ::FindItem(m_plgBook->GetTreeCtrl(), m_plgBook->GetTreeCtrl()->GetRootItem(), label);
            if (current.IsOk()) {
                wxMessageBox(_("Plugin already exist in this profile"), _("Warning"), wxOK | wxICON_WARNING, this);
            }
            else {
                // add to list
                wiPlgTreeData* data = (wiPlgTreeData*)dlg.m_plgTree->GetItemData(dlg.m_selected);

                if (data != NULL && data->plg != NULL)
                {
                    (void) LoadPluginSettings(data->plg);
                } // end of wiPlgTreeData processing
            } // end of addition plugin to list
        } // end of ShowModal() == wxOK
    } // end of m_plugList != NULL
    else {
        wxLogError(wxT("Can't get plugin list!"));
    }
}

void wiMainForm::OnRemovePlugin( wxCommandEvent& event )
{
    int idx = m_plgBook->GetSelection();
    int img = m_plgBook->GetPageImage(idx);
    if (img > 4) {
        m_plgBook->DeletePage(idx);
    }
}

PluginInfo* wiMainForm::FoundPlugin( const wxString& id)
{
    PluginInfo* retval = NULL;

    if (m_plugList != NULL) {
        for (size_t lstSize = 0; lstSize < m_plugList->size(); lstSize++) {
            wxString plgId = FromStdString((*m_plugList)[lstSize].PluginId);
            if (id == plgId) {
                retval = &((*m_plugList)[lstSize]);
                break;
            }
        }
    }
    return retval;
}

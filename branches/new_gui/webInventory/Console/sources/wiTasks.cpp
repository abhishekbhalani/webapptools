#include <wx/wupdlock.h>
#include "wiTasks.h"
#include "wiMainForm.h"
#include "wiPluginSettings.h"
#include "wiObjDialog.h"
#include "wiPlgSelector.h"
#include "treeData.h"
#include "messages.h"
#include "optionnames.h"

// wxTreebook images
#include "../images/opts_global.xpm"
#include "../images/opts_transport.xpm"
#include "../images/opts_inventory.xpm"
#include "../images/opts_audit.xpm"
#include "../images/opts_vulner.xpm"

// object toolbar images
#include "../images/_btnAdd.xpm"
#include "../images/_btnDel.xpm"
#include "../images/_btnEdit.xpm"

// profile toolbar images
// #include "../images/_btnAdd.xpm"
#include "../images/_btnCopy.xpm"
//#include "../images/_btnDel.xpm"
#include "../images/_flsave.xpm"
#include "../images/_start.xpm"

// task toolbar images
#include "../images/start.xpm"
//#include "../images/_start.xpm"
#include "../images/pause.xpm"
#include "../images/_pause.xpm"

// tasklist images
#include "../images/tree_unk.xpm"
// #include "../images/start.xpm"
#include "../images/btnStop.xpm"
#include "../images/_btnStop.xpm"
#include "../images/_reload.xpm"
// #include "../images/_flsave.xpm"

static wxString genericSettings = wxT("<plugin>\
  <category label=\"Basic settings\" name=\"generic\">\
    <option name=\"LogLevel\" label=\"Logging Level\" type=\"2\" control=\"enum\">Trace;Debug;Info;Warning;Error;Fatal</option>\
    <option name=\"TaskTail\" label=\"Task log tail\" type=\"2\" control=\"int\" min=\"0\" max=\"100\">10</option>\
  </category>\
</plugin>");

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

static wxTreeItemId FindTreeItem(wxTreeCtrl* tree, wxTreeItemId curr, const wxString& label)
{
    if (tree->GetItemText(curr) == label) {
        return curr;
    }
    wxTreeItemId child, search;
    wxTreeItemIdValue cookie;

    child = tree->GetFirstChild(curr, cookie);
    while (child.IsOk()) {
        search = FindTreeItem(tree, child, label);
        if (search.IsOk()) {
            break;
        }
        child = tree->GetNextChild(curr, cookie);
    }

    return search;
}

wiTasks::wiTasks( wxWindow* parent ) :
    Tasks( parent ),
    m_lstImages(16, 16)
{
    wxArrayString labels;

    // remove design-time element
	bSzProfile->Remove(m_plgBook);
    // delete m_plgBook; // can't delete to avoid crash on events disconnecting

    // create wxTreebook control
    m_plgBookTree = new wxTreebook( m_panObjects, wxID_ANY, wxDefaultPosition, wxDefaultSize );
	wxSize m_plgBookImageSize = wxSize( 16,16 );
	int m_plgBookIndex = 0;
	wxImageList* m_plgBookImages = new wxImageList( m_plgBookImageSize.GetWidth(), m_plgBookImageSize.GetHeight() );
	m_plgBookTree->AssignImageList( m_plgBookImages );

	wxPanel* generic = new wiPluginSettings(m_plgBookTree, genericSettings);
	m_plgBookTree->AddPage( generic, _("Generic options"), true );

	bSzProfile->Add( m_plgBookTree, 1, wxEXPAND | wxALL, 0 );

	RebuildTreeView();

    m_plgBookTree->Connect( wxEVT_COMMAND_TREEBOOK_PAGE_CHANGING, wxTreebookEventHandler( wiTasks::OnOptionsPageChanging ), NULL, this );

    // fullfill ImageList
    m_lstImages.Add(wxIcon(tree_unk_xpm));
    m_lstImages.Add(wxIcon(start_xpm));
    m_lstImages.Add(wxIcon(btnStop_xpm));

    // create layout for tasks list
    m_lstTaskList->InsertColumn(0, wxT(""), wxLIST_FORMAT_LEFT, 20);
    m_lstTaskList->InsertColumn(1, _("Task name"), wxLIST_FORMAT_LEFT, 180);
    m_lstTaskList->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, 96);
    m_lstTaskList->SetImageList(&m_lstImages, wxIMAGE_LIST_SMALL);
    m_lstTaskList->DeleteAllItems();

    // create layout for objects list
    m_lstObjectList->InsertColumn(0, wxT("Object name"), wxLIST_FORMAT_LEFT, 180);
    m_lstObjectList->DeleteAllItems();

    // fix tolbar images - add "disabled"
    m_toolBarObject->SetToolDisabledBitmap(wxID_TOOLNEW, wxBitmap(_btnAdd_xpm));
    m_toolBarObject->SetToolDisabledBitmap(wxID_TOOLEDIT, wxBitmap(_btnEdit_xpm));
    m_toolBarObject->SetToolDisabledBitmap(wxID_TOOLDEL, wxBitmap(_btnDel_xpm));

    m_toolBarProf->SetToolDisabledBitmap(wxID_TLPROFNEW, wxBitmap(_btnAdd_xpm));
    m_toolBarProf->SetToolDisabledBitmap(wxID_TLPROFCLONE, wxBitmap(_btnCopy_xpm));
    m_toolBarProf->SetToolDisabledBitmap(wxID_TLPROFDEL, wxBitmap(_btnDel_xpm));
    m_toolBarProf->SetToolDisabledBitmap(wxID_TLPROFSAVE, wxBitmap(_flsave_xpm));
    m_toolBarProf->SetToolDisabledBitmap(wxID_TOOLGO, wxBitmap(_start_xpm));

    m_toolBarTasks->SetToolDisabledBitmap(wxID_TOOLPAUSE, wxBitmap(_start_xpm));
    m_toolBarTasks->SetToolDisabledBitmap(wxID_TOOLSTOP, wxBitmap(_btnStop_xpm));
    m_toolBarTasks->SetToolDisabledBitmap(wxID_TLREFRTSKLOG, wxBitmap(_reload_xpm));
    m_toolBarTasks->SetToolDisabledBitmap(wxID_TLSAVETSKLOG, wxBitmap(_flsave_xpm));

	m_selectedObject = -1;
	m_selectedTask = -1;
	m_selectedProf = -1;

    Disable();
}

void wiTasks::RebuildTreeView()
{
    wxImageList* icons;
    wxPanel *panel;

    wxWindowUpdateLocker taskList(m_plgBookTree);

    m_plgBookTree->GetTreeCtrl()->SetWindowStyle(wxTR_FULL_ROW_HIGHLIGHT|wxTR_HAS_BUTTONS|wxTR_HIDE_ROOT|wxTR_LINES_AT_ROOT|wxTR_SINGLE);

    icons = new wxImageList(16, 16);
    icons->RemoveAll();
    icons->Add(wxBitmap(opts_global_xpm));
    icons->Add(wxBitmap(opts_transport_xpm));
    icons->Add(wxBitmap(opts_inventory_xpm));
    icons->Add(wxBitmap(opts_audit_xpm));
    icons->Add(wxBitmap(opts_vulner_xpm));
    m_plgBookTree->AssignImageList(icons);

    while (m_plgBookTree->GetPageCount() > 1) {
        m_plgBookTree->DeletePage(1);
    }
    m_plgBookTree->SetPageImage(0, 0);
    // transports
    panel = new wiPluginSettings(m_plgBookTree, wxT(""));
    m_plgBookTree->AddPage(panel, _("Transports"), false, 1);
    // inventory
    panel = new wiPluginSettings(m_plgBookTree, wxT(""));
    m_plgBookTree->AddPage(panel, _("Inventory"), false, 2);
    // audit
    panel = new wiPluginSettings(m_plgBookTree, wxT(""));
    m_plgBookTree->AddPage(panel, _("Audit"), false, 3);
    // vulners
    panel = new wiPluginSettings(m_plgBookTree, wxT(""));
    m_plgBookTree->AddPage(panel, _("Vulnerabilities"), false, 4);

    m_plgBookTree->SetSelection(0);
}

wxPanel* wiTasks::CreateDefaultPanel(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent);
    (void) new wxStaticText( panel, wxID_ANY, _("This plugin doesn't provide any settings"), wxDefaultPosition, wxDefaultSize, 0 );
    return panel;
}

void wiTasks::OnOptionsPageChanging( wxTreebookEvent& event )
{
    int idx = event.GetSelection();
    idx = m_plgBookTree->GetPageImage(idx);
    if (idx >= 1 && idx <= 4) {
        event.Veto();
    }
    else {
        // do something for page changing
    }
}

void wiTasks::OnAddObject( wxCommandEvent& event )
{
    wiObjDialog objDlg(this);
    wxString name, url;
    ObjectInf objInfo;

    if (objDlg.ShowModal() == wxOK) {
        // save new object
        objInfo.ObjectId = "0";
        objInfo.Name = objDlg.m_txtObjName->GetValue().utf8_str();
        objInfo.Address = objDlg.m_txtBaseURL->GetValue().utf8_str();
        FRAME_WINDOW->UpdateObject(objInfo);
        ProcessObjects();
    }
}

void wiTasks::OnEditObject( wxCommandEvent& event )
{
    wiObjDialog objDlg(this);
    wxString name, url;
    ObjectInf* obj;

    if (m_selectedObject > -1) {
        obj = (ObjectInf*)m_lstObjectList->GetItemData(m_selectedObject);
        objDlg.m_txtObjName->SetValue(FRAME_WINDOW->FromStdString(obj->Name));
        objDlg.m_txtBaseURL->SetValue(FRAME_WINDOW->FromStdString(obj->Address));
        if (objDlg.ShowModal() == wxOK) {
            // save new object
            obj->Name = objDlg.m_txtObjName->GetValue().utf8_str();
            obj->Address = objDlg.m_txtBaseURL->GetValue().utf8_str();
            FRAME_WINDOW->UpdateObject(*obj);
            //ProcessObjects();
        }
    }
}

void wiTasks::OnDelObject( wxCommandEvent& event )
{
    wxString name;
    wxListItem info;
    ObjectInf* obj;

    if (m_selectedObject > -1)
    {
        info.SetId(m_selectedObject);
        info.SetColumn(0);
        info.SetMask(wxLIST_MASK_TEXT);
        m_lstObjectList->GetItem(info);
        name = wxString::Format(_("Are you sure to delete object '%s'?"), info.GetText().c_str());
        int res = wxMessageBox(name, _("Confirm"), wxYES_NO | wxICON_QUESTION, this);
        if (res == wxYES) {
            obj = (ObjectInf*)m_lstObjectList->GetItemData(m_selectedObject);
            FRAME_WINDOW->DoClientCommand(wxT("delobj"), FRAME_WINDOW->FromStdString(obj->ObjectId));
            ProcessObjects();
        }
    }
}

void wiTasks::OnTaskKillFocus( wxFocusEvent& event )
{
	// TODO: Implement OnTaskKillFocus
}

void wiTasks::OnSelectObject( wxListEvent& event )
{
    m_selectedObject = event.GetIndex();
    ControlStatus();
}

void wiTasks::OnChangeProfile( wxCommandEvent& event )
{
    wxListItem info;
    int prevID = m_selectedProf;
    ProfileInf* dat;

    m_selectedProf = m_chProfile->GetSelection();
    ControlStatus();

    if (m_selectedProf != prevID && m_selectedTask > -1) {
        // check options changing and ask for save them
    }
    if (m_selectedProf > -1) {
        m_bpAddPlugin->Enable(true);
        m_bpDelPlugin->Enable(true);
        dat = (ProfileInf*)m_chProfile->GetClientData(m_selectedProf);
        m_txtProfName->SetValue(FRAME_WINDOW->FromStdString(dat->Name));
        //LoadProfileOptions(FromStdString(dat->ObjectId));
    }
    else {
        m_bpAddPlugin->Enable(false);
        m_bpDelPlugin->Enable(false);
    }
}

void wiTasks::OnAddProfile( wxCommandEvent& event )
{
    wxString name;

    name = wxGetTextFromUser(_("Input new profile name"), _("Query"), wxT(""), this);
    if (!name.IsEmpty() && FRAME_WINDOW->IsConnected()) {
        name = FRAME_WINDOW->DoClientCommand(wxT("addprofile"), name);
    }
    ProcessProfileList();
}

void wiTasks::OnCopyProfile( wxCommandEvent& event )
{
    ProfileList* lst;
    ProfileInf* dat;
    int selected;
    wxString idSelect;
    wxString idPrev;
    wxString xml;

    selected = m_chProfile->GetSelection();
    if (selected > -1) {
        dat = (ProfileInf*)m_chProfile->GetClientData(selected);
        idSelect = FRAME_WINDOW->FromStdString(dat->Name) + wxT(" copy");
        if(FRAME_WINDOW->IsConnected()) {
            idSelect = FRAME_WINDOW->DoClientCommand(wxT("addprofile"), idSelect);
            if (idSelect != wxT("") && idSelect != wxT("0")) {
                ProcessProfileList();
                idSelect = FRAME_WINDOW->FromStdString(dat->Name) + wxT(" copy");
                selected = m_chProfile->FindString(idSelect);
                m_selectedProf = selected;
                idPrev = m_txtProfName->GetValue();
                m_txtProfName->SetValue(idSelect);
                OnTaskApply(event);
                m_txtProfName->SetValue(idPrev);
            }
        }
    }
}

void wiTasks::OnDelProfile( wxCommandEvent& event )
{
    wxString name;
    ProfileInf* dat;
    int selected;

    selected = m_chProfile->GetSelection();
    if (selected > -1)
    {
        name = m_chProfile->GetString(selected);
        dat = (ProfileInf*)m_chProfile->GetClientData(selected);
        name = wxString::Format(_("Are you sure to delete profile '%s'?"), name.c_str());
        int res = wxMessageBox(name, _("Confirm"), wxYES_NO | wxICON_QUESTION, this);
        if (res == wxYES) {
            name = FRAME_WINDOW->FromStdString(dat->ObjectId);
            FRAME_WINDOW->DoClientCommand(wxT("delprofile"), name);
            ProcessProfileList();
        }
    }
}

void wiTasks::OnTaskApply( wxCommandEvent& event )
{
	// TODO: Implement OnTaskApply
}

void wiTasks::OnRunTask( wxCommandEvent& event )
{
    // save profile settings
    OnTaskApply( event );
	// TODO: Implement OnRunTask
}

void wiTasks::OnAddPlugin( wxCommandEvent& event )
{
    size_t lstSize, i;
    wxTreeItemId parent, current;
    wiPlgSelector dlg(this);
    PluginList* plugList;

    plugList = FRAME_WINDOW->GetPluginList();

    dlg.m_plgTree->DeleteAllItems();
    if (plugList != NULL) {
        dlg.m_plgTree->AddRoot(wxT("iwePlugin"));
        for (lstSize = 0; lstSize < plugList->size(); lstSize++) {
            parent = dlg.m_plgTree->GetRootItem();
            for (i = 0; i < (*plugList)[lstSize].IfaceList.size(); i++)
            {
                wxString label = FRAME_WINDOW->FromStdString((*plugList)[lstSize].IfaceList[i]);
                current = FindTreeItem(dlg.m_plgTree, parent, label);

                if (current.IsOk()) {
                    parent = current;
                    if ((i + 1) == (*plugList)[lstSize].IfaceList.size()) {
                        wiPlgTreeData* data = new wiPlgTreeData(&((*plugList)[lstSize]));
                        dlg.m_plgTree->SetItemData(current, data);
                    }
                }
                else {
                    wiPlgTreeData* data = new wiPlgTreeData(&((*plugList)[lstSize]));
                    parent = dlg.m_plgTree->AppendItem(parent, label, -1, -1, data);
                }
            }
        }
        dlg.m_plgTree->ExpandAll();
        if (dlg.ShowModal() == wxOK) {
            wxString label = dlg.m_plgTree->GetItemText(dlg.m_selected);
            current = FindTreeItem(m_plgBookTree->GetTreeCtrl(), m_plgBookTree->GetTreeCtrl()->GetRootItem(), label);
            if (current.IsOk()) {
                wxMessageBox(_("Plugin already exist in this profile"), _("Warning"), wxOK | wxICON_WARNING, this);
            }
            else {
                // add to list
                wiPlgTreeData* data = (wiPlgTreeData*)dlg.m_plgTree->GetItemData(dlg.m_selected);

                if (data != NULL && data->plg != NULL)
                {
                    PluginInf* plg = data->plg;
                    char** xpm = FRAME_WINDOW->StringListToXpm((vector<string>&)plg->PluginIcon);
                    int imgIdx = m_plgBookTree->GetImageList()->Add(wxBitmap(xpm));
                    wxString pageLabel;
                    int pageIdx = 0;
                    wiPluginSettings* bookPage;
                    wxString xrc;

                    if (find(plg->IfaceList.begin(), plg->IfaceList.end(), "iTransport") != plg->IfaceList.end())
                    {   // add plugins to the transport list
                        pageLabel = _("Transports");
                    }
                    else if (find(plg->IfaceList.begin(), plg->IfaceList.end(), "iInventory") != plg->IfaceList.end())
                    {   // add plugins to the inventory list
                        pageLabel = _("Inventory");
                    }
                    else if (find(plg->IfaceList.begin(), plg->IfaceList.end(), "iAudit") != plg->IfaceList.end())
                    {   // add plugins to the audit list
                        pageLabel = _("Audit");
                    }
                    else if (find(plg->IfaceList.begin(), plg->IfaceList.end(), "iVulner") != plg->IfaceList.end())
                    {   // add plugins to the vulnerabilities list
                        pageLabel = _("Vulnerabilities");
                    }
                    else
                    {   // add plugins to the generic list
                        pageLabel = _("Generic options");
                    }
                    for (int i = 0; i < m_plgBookTree->GetPageCount(); i++) {
                        if (m_plgBookTree->GetPageText(i) == pageLabel) {
                            pageIdx = i;
                            break;
                        }
                    }
                    pageLabel = FRAME_WINDOW->FromStdString(plg->IfaceList.back());
                    if (FRAME_WINDOW->IsConnected()) {
                        xrc = FRAME_WINDOW->DoClientCommand(wxT("plgui"), FRAME_WINDOW->FromStdString(plg->PluginId));
                    }
                    else {
                        xrc = wxEmptyString;
                    }

                    bookPage = new wiPluginSettings(m_plgBookTree, xrc);
                    if (bookPage) {
                        bookPage->SetName(FRAME_WINDOW->FromStdString(plg->PluginId));
                        m_plgBookTree->InsertSubPage(pageIdx, bookPage, pageLabel, false, imgIdx);
                    }
                    else {
                        wxLogError(_("Can't create UI for plugin %s"), FRAME_WINDOW->FromStdString(plg->PluginDesc));
                    }
                    // load actual data into UI
                } // end of wiPlgTreeData processing
            } // end of addition plugin to list
        } // end of ShowModal() == wxOK
    } // end of m_plugList != NULL
    else {
        wxLogError(wxT("Can't get plugin list!"));
    }
}

void wiTasks::OnRemovePlugin( wxCommandEvent& event )
{
    int idx = m_plgBookTree->GetSelection();
    int img = m_plgBookTree->GetPageImage(idx);
    if (img > 4) {
        m_plgBookTree->DeletePage(idx);
    }
}

void wiTasks::OnPauseTask( wxCommandEvent& event )
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
            FRAME_WINDOW->DoClientCommand(wxT("runtask"), name);
        }
        else if (info.GetImage() == WI_TSK_RUN) {
            FRAME_WINDOW->DoClientCommand(wxT("pausetask"), name);
        }

        ProcessTaskList();
        ControlStatus();
    }
}

void wiTasks::OnCancelTask( wxCommandEvent& event )
{
    wxListItem info;

    if (m_selectedTask > -1)
    {
        int id = m_lstTaskList->GetItemData(m_selectedTask);
        wxString name = wxString::Format(wxT("%X"), id);
        FRAME_WINDOW->DoClientCommand(wxT("canceltask"), name);

        ProcessTaskList();
        ControlStatus();
    }
}

void wiTasks::OnSortItems( wxListEvent& event )
{
    sortOrder = ~sortOrder;
    m_lstTaskList->SortItems(SortItemFunc, (long)m_lstTaskList);
}

void wiTasks::OnTaskSelected( wxListEvent& event )
{
    m_selectedTask = event.GetIndex();
    ControlStatus();
    /// @todo Reload task log
}

void wiTasks::Connected(bool forced)
{
    Enable();
    ProcessObjects();
    ProcessProfileList();
    ProcessTaskList();
}

void wiTasks::Disconnected(bool forced)
{
    Disable();
    if (forced) {
        // clear data
        m_lstObjectList->DeleteAllItems();
        m_chProfile->Clear();
        RebuildTreeView();
        m_lstTaskList->DeleteAllItems();
        m_rtTask->Clear();
    }
}

void wiTasks::ProcessObjects(const wxString& criteria /*= wxT("")*/)
{
    ObjectList* lst;
    size_t lstSize;
    int idx = 0;

    lst = FRAME_WINDOW->GetObjectList(criteria);
    if (lst != NULL) {
        wxWindowUpdateLocker taskList(m_lstObjectList);

        m_lstObjectList->DeleteAllItems();
        for (lstSize = 0; lstSize < lst->size(); lstSize++) {
            wxString idStr = FRAME_WINDOW->FromStdString((*lst)[lstSize].Name.c_str());
            idx = m_lstObjectList->GetItemCount();
            m_lstObjectList->InsertItem(idx, idStr);
            m_lstObjectList->SetItemData(idx, (wxUIntPtr)(&((*lst)[lstSize])));
        }

        /// @todo for report view - FillObjectFilter();
        if (m_selectedObject >= m_lstObjectList->GetItemCount()) {
            m_selectedObject = m_lstObjectList->GetItemCount() - 1;
        }
        m_lstObjectList->SetItemState(m_selectedObject, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    else {
        FRAME_WINDOW->ShowConnectionError();
    }
    ControlStatus();
}

void wiTasks::ProcessProfileList(const wxString& criteria /*= wxT("")*/)
{
    ProfileList* lst;
    ProfileInf* dat;
    size_t lstSize;
    int idx = 0;
    int selected;
    wxString idSelect = wxT("0");

    selected = m_chProfile->GetSelection();
    if (selected > -1) {
        dat = (ProfileInf*)m_chProfile->GetClientData(selected);
        idSelect = FRAME_WINDOW->FromStdString(dat->ObjectId);
    }
    lst = FRAME_WINDOW->GetProfileList(criteria);
    if (lst != NULL) {
        wxWindowUpdateLocker taskList(m_chProfile);

        m_chProfile->Clear();
        selected = -1;
        for (lstSize = 0; lstSize < lst->size(); lstSize++) {
            wxString idStr = FRAME_WINDOW->FromStdString((*lst)[lstSize].Name);
            idx = m_chProfile->GetCount();
            idx = m_chProfile->Insert(idStr, idx, (void*)(&((*lst)[lstSize])));
            idStr = FRAME_WINDOW->FromStdString((*lst)[lstSize].ObjectId);
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
        FRAME_WINDOW->ShowConnectionError();
    }
    ControlStatus();
}

void wiTasks::ProcessTaskList(const wxString& criteria /*= wxT("")*/)
{
    TaskList* lst;
    size_t lstSize;
    int idx = 0;
    long idLong;

    lst = FRAME_WINDOW->GetTaskList(criteria);
    if (lst != NULL) {
        wxWindowUpdateLocker taskList(m_lstTaskList);

        m_lstTaskList->DeleteAllItems();
        for (lstSize = 0; lstSize < lst->size(); lstSize++) {
            wxString idStr = FRAME_WINDOW->FromStdString((*lst)[lstSize].id.c_str());
            idStr.ToLong(&idLong, 16);
            idx = m_lstTaskList->GetItemCount();
            m_lstTaskList->InsertItem(idx, (*lst)[lstSize].status);
            m_lstTaskList->SetItem(idx, 1, FRAME_WINDOW->FromStdString((*lst)[lstSize].name));
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
        FRAME_WINDOW->ShowConnectionError();
    }
    ControlStatus();
}

void wiTasks::ControlStatus()
{
    wxListItem info;

    if (m_selectedObject == -1) {
        m_toolBarObject->EnableTool(wxID_TOOLNEW, true);
        m_toolBarObject->EnableTool(wxID_TOOLEDIT, false);
        m_toolBarObject->EnableTool(wxID_TOOLDEL, false);
    }
    else {
        m_toolBarObject->EnableTool(wxID_TOOLNEW, true);
        m_toolBarObject->EnableTool(wxID_TOOLEDIT, true);
        m_toolBarObject->EnableTool(wxID_TOOLDEL, true);
    }

    if (m_chProfile->GetSelection() == -1) {
        m_toolBarProf->EnableTool(wxID_TLPROFNEW, true);
        m_toolBarProf->EnableTool(wxID_TLPROFCLONE, false);
        m_toolBarProf->EnableTool(wxID_TLPROFDEL, false);
        m_toolBarProf->EnableTool(wxID_TLPROFSAVE, false);
        m_toolBarProf->EnableTool(wxID_TOOLGO, false);
        m_bpAddPlugin->Disable();
        m_bpDelPlugin->Disable();
    }
    else {
        m_toolBarProf->EnableTool(wxID_TLPROFNEW, true);
        m_toolBarProf->EnableTool(wxID_TLPROFCLONE, true);
        m_toolBarProf->EnableTool(wxID_TLPROFDEL, true);
        m_toolBarProf->EnableTool(wxID_TLPROFSAVE, true);
        m_bpAddPlugin->Enable();
        m_bpDelPlugin->Enable();
        if (m_selectedObject > -1) {
            m_toolBarProf->EnableTool(wxID_TOOLGO, true);
        }
    }

    if (m_selectedTask == -1) {
        m_toolBarTasks->Disable();
    }
    else {
        m_toolBarTasks->Enable();
        // change buttons bitmaps on depends of task status
        int id = m_lstTaskList->GetItemData(m_selectedTask);
        info.SetId(m_selectedTask);
        info.SetColumn(0);
        info.SetMask(wxLIST_MASK_IMAGE);
        m_lstTaskList->GetItem(info);
        if (info.GetImage() == WI_TSK_IDLE || info.GetImage() == WI_TSK_PAUSED) {
            m_toolBarTasks->SetToolNormalBitmap(wxID_TOOLPAUSE, wxBitmap(start_xpm));
            m_toolBarTasks->SetToolDisabledBitmap(wxID_TOOLPAUSE, wxBitmap(_start_xpm));
        }
        else if (info.GetImage() == WI_TSK_RUN) {
            m_toolBarTasks->SetToolNormalBitmap(wxID_TOOLPAUSE, wxBitmap(pause_xpm));
            m_toolBarTasks->SetToolDisabledBitmap(wxID_TOOLPAUSE, wxBitmap(_pause_xpm));
        }
    }
}

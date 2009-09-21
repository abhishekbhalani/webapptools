#include <wx/wupdlock.h>
#include "wiTasks.h"
#include "wiPluginSettings.h"

// wxTreebook images
#include "../images/opts_global.xpm"
#include "../images/opts_transport.xpm"
#include "../images/opts_inventory.xpm"
#include "../images/opts_audit.xpm"
#include "../images/opts_vulner.xpm"

// tasklist images
#include "../images/tree_unk.xpm"
#include "../images/start.xpm"
#include "../images/btnStop.xpm"

static wxString genericSettings = wxT("<plugin>\
  <category label=\"Basic settings\" name=\"generic\">\
    <option name=\"LogLevel\" label=\"Logging Level\" type=\"2\" control=\"enum\">Trace;Debug;Info;Warning;Error;Fatal</option>\
    <option name=\"TaskTail\" label=\"Task log tail\" type=\"2\" control=\"int\" min=\"0\" max=\"100\">10</option>\
  </category>\
</plugin>");

wiTasks::wiTasks( wxWindow* parent ) :
    Tasks( parent ),
    m_lstImages(16, 16)
{
    wxArrayString labels;

    // remove design-time element
	bSzProfile->Remove(m_plgBook);
    delete m_plgBook;

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
    idx = m_plgBook->GetPageImage(idx);
    if (idx >= 1 && idx <= 4) {
        event.Veto();
    }
    else {
        // do something for page changing
    }
}

void wiTasks::OnAddObject( wxCommandEvent& event )
{
	// TODO: Implement OnAddObject
}

void wiTasks::OnEditObject( wxCommandEvent& event )
{
	// TODO: Implement OnEditObject
}

void wiTasks::OnDelObject( wxCommandEvent& event )
{
	// TODO: Implement OnDelObject
}

void wiTasks::OnTaskKillFocus( wxFocusEvent& event )
{
	// TODO: Implement OnTaskKillFocus
}

void wiTasks::OnSelectObject( wxListEvent& event )
{
	// TODO: Implement OnSelectObject
}

void wiTasks::OnChangeProfile( wxCommandEvent& event )
{
	// TODO: Implement OnChangeProfile
}

void wiTasks::OnAddProfile( wxCommandEvent& event )
{
	// TODO: Implement OnAddProfile
}

void wiTasks::OnCopyProfile( wxCommandEvent& event )
{
	// TODO: Implement OnCopyProfile
}

void wiTasks::OnDelProfile( wxCommandEvent& event )
{
	// TODO: Implement OnDelProfile
}

void wiTasks::OnTaskApply( wxCommandEvent& event )
{
	// TODO: Implement OnTaskApply
}

void wiTasks::OnRunTask( wxCommandEvent& event )
{
	// TODO: Implement OnRunTask
}

void wiTasks::OnAddPlugin( wxCommandEvent& event )
{
	// TODO: Implement OnAddPlugin
}

void wiTasks::OnRemovePlugin( wxCommandEvent& event )
{
	// TODO: Implement OnRemovePlugin
}

void wiTasks::OnPauseTask( wxCommandEvent& event )
{
	// TODO: Implement OnPauseTask
}

void wiTasks::OnCancelTask( wxCommandEvent& event )
{
	// TODO: Implement OnCancelTask
}

void wiTasks::OnSortItems( wxListEvent& event )
{
	// TODO: Implement OnSortItems
}

void wiTasks::OnTaskSelected( wxListEvent& event )
{
	// TODO: Implement OnTaskSelected
}

void wiTasks::Connected(bool forced)
{
}

void wiTasks::Disconnected(bool forced)
{
}

#include "wiTasks.h"

wiTasks::wiTasks( wxWindow* parent )
:
Tasks( parent )
{
    wxArrayString labels;

    // !DEBUG +++
    labels.Add(wxT("Trace"));
    labels.Add(wxT("Debug"));
    labels.Add(wxT("Info"));
    labels.Add(wxT("Warning"));
    labels.Add(wxT("Error"));
    labels.Add(wxT("Fatal"));
    m_pgridGeneric->Append( new wxPropertyCategory(wxT("Generic options"), wxT("generic")));
    m_pgridGeneric->Append(new wxEnumProperty(wxT("Loging level"), wxPG_LABEL, labels) );
    m_pgridGeneric->Append( new wxIntProperty(wxT("Task log tail"), wxPG_LABEL, 10) );
//    m_pgridGeneric->SetPropertyAttribute(wxT("Task log tail"), wxPG_ATTR_MIN, (long)0 );
//    m_pgridGeneric->SetPropertyAttribute(wxT("Task log tail"), wxPG_ATTR_MAX, (long)1000 );
//    m_pgridGeneric->SetPropertyAttribute(wxT("Task log tail"), wxPG_ATTR_UNITS, wxT("lines") );
    m_pgridGeneric->CollapseAll();
    m_pgridGeneric->ExpandAll();
    // !DEBUG ---
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

void wiTasks::OnOptionsPageChanging( wxListbookEvent& event )
{
	// TODO: Implement OnOptionsPageChanging
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

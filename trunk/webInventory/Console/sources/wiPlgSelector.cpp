#include "wiPlgSelector.h"
#include <wx/msgdlg.h>

wiPlgSelector::wiPlgSelector( wxWindow* parent )
:
PlgSelector( parent )
{
}

void wiPlgSelector::OnOK( wxCommandEvent& event )
{
	m_selected = m_plgTree->GetSelection();
	if (m_selected.IsOk()) {
	    EndModal(wxOK);
	}
	else {
	    ::wxMessageBox(_("Select plugin to add"), _("Warning"), wxOK | wxICON_WARNING, this);
	}
}

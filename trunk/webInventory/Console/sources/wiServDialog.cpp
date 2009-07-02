#include "wiServDialog.h"

wiServDialog::wiServDialog( wxWindow* parent )
:
ServDialog( parent )
{

}

void wiServDialog::OnOK( wxCommandEvent& event )
{
	EndModal(wxOK);
}

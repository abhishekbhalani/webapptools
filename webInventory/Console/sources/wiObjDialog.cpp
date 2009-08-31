#include "wiObjDialog.h"

wiObjDialog::wiObjDialog( wxWindow* parent )
:
ObjDialog( parent )
{

}

void wiObjDialog::OnOK( wxCommandEvent& event )
{
	EndModal(wxOK);
}

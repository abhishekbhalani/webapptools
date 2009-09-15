#include <wx/dcmemory.h>
#include "wiSettings.h"
#include "../images/opts_vulner.xpm"

wiSettings::wiSettings( wxWindow* parent )
:
Settings( parent )
{
    InitPluginsGrid();
}

void wiSettings::InitPluginsGrid()
{
    wxPropertyGridPage* pg;
    wxBitmap* bmp;
    wxArrayString labels;
    wxSize bmpSize;
    wxMemoryDC mdc;
    int x, y;

    // !DEBUG +++
    labels.Add(wxT("Interface 1"));
    labels.Add(wxT("Interface 2"));
    // !DEBUG ---

    m_pgrdManager->Clear();
    m_pgrdManager->AddPage(wxT("PluginInfo"));

    pg = m_pgrdManager->GetPage(wxT("PluginInfo"));
    pg->Append( new wxPropertyCategory(wxT("Generic plugins"), wxT("generic")));
    // !DEBUG +++
    pg->Append(new wxStringProperty(wxT("Plugin 1"),wxPG_LABEL,wxT("Interface")) );
    pg->Append(new wxEnumProperty(wxT("Plugin 2"),wxPG_LABEL, labels) );
    bmpSize = m_pgrdManager->GetGrid()->GetImageSize();
    x = bmpSize.GetWidth();
    y = bmpSize.GetHeight() ;
    bmp = new wxBitmap(x, y, 32);
    x -= 16;
    x /= 2;
    if (x > 0) {
        x = 0;
    }
    y -= 16;
    y /= 2;
    if (y > 0) {
        y = 0;
    }
    mdc.SelectObject(*bmp);
    mdc.Clear();
    mdc.DrawBitmap(opts_vulner_xpm, x, y, true);
    mdc.SelectObject(wxNullBitmap);
    pg->SetPropertyImage( wxT("Plugin 2"), *bmp );
    // !DEBUG ---
    pg->Append( new wxPropertyCategory(_("Transport plugins"), wxT("transport")));
    // !DEBUG +++
    pg->Append(new wxStringProperty(wxT("Plugin 3"),wxPG_LABEL,wxT("Interface")) );
    // !DEBUG ---
    pg->Append( new wxPropertyCategory(_("Inventory plugins"), wxT("inventory")));
    // !DEBUG +++
    pg->Append(new wxStringProperty(wxT("Plugin 4"),wxPG_LABEL,wxT("Interface")) );
    // !DEBUG ---
    pg->Append( new wxPropertyCategory(_("Audit plugins"), wxT("audit")));
    // !DEBUG +++
    pg->Append(new wxStringProperty(wxT("Plugin 5"),wxPG_LABEL,wxT("Interface")) );
    // !DEBUG ---
    pg->Append( new wxPropertyCategory(_("Vulnerabilities detection plugins"), wxT("vulner")));
    // !DEBUG +++
    pg->Append(new wxStringProperty(wxT("Plugin 6"),wxPG_LABEL,wxT("Interface")) );
    // !DEBUG ---
    fgSrvSizer->Layout();
    m_pgrdManager->CollapseAll();
    m_pgrdManager->ExpandAll();
}

void wiSettings::OnConnect( wxCommandEvent& event )
{
	// TODO: Implement OnConnect
}

void wiSettings::OnAddServer( wxCommandEvent& event )
{
	// TODO: Implement OnAddServer
}

void wiSettings::OnEditServer( wxCommandEvent& event )
{
	// TODO: Implement OnEditServer
}

void wiSettings::OnDelServer( wxCommandEvent& event )
{
	// TODO: Implement OnDelServer
}

void wiSettings::OnLangChange( wxCommandEvent& event )
{
	// TODO: Implement OnLangChange
}

void wiSettings::OnStorageChange( wxCommandEvent& event )
{
	// TODO: Implement OnStorageChange
}

void wiSettings::OnPlgRefresh( wxCommandEvent& event )
{
	// TODO: Implement OnPlgRefresh
}

void wiSettings::OnSrvStop( wxCommandEvent& event )
{
	// TODO: Implement OnSrvStop
}

void wiSettings::OnServerLogs( wxCommandEvent& event )
{
	// TODO: Implement OnServerLogs
}

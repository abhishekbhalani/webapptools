///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wiStatBar.h"

#include "wiGuiData.h"

#include "../images/btnAdd.xpm"
#include "../images/btnDel.xpm"
#include "../images/panReports.xpm"
#include "../images/panSettings.xpm"
#include "../images/panTasks.xpm"
#include "../images/pause.xpm"
#include "../images/start.xpm"

///////////////////////////////////////////////////////////////////////////

MainForm::MainForm( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 800,600 ), wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	m_mainnb = new wxAuiNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_SCROLL_BUTTONS|wxAUI_NB_TAB_FIXED_WIDTH );
	m_pTasks = new wxPanel( m_mainnb, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->AddGrowableRow( 3 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText1 = new wxStaticText( m_pTasks, wxID_ANY, wxT("Active tasks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	m_staticText1->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer4->Add( m_staticText1, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );

	m_bpButton51 = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( start ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpButton51->Enable( false );

	m_bpButton51->Enable( false );

	bSizer4->Add( m_bpButton51, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_bpButton511 = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( pause ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpButton511->Enable( false );

	m_bpButton511->Enable( false );

	bSizer4->Add( m_bpButton511, 0, wxALIGN_CENTER_VERTICAL, 0 );

	m_bpButton2 = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( btnDel ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpButton2->Enable( false );

	m_bpButton2->Enable( false );

	bSizer4->Add( m_bpButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	fgSizer1->Add( bSizer4, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText2 = new wxStaticText( m_pTasks, wxID_ANY, wxT("Task execution log"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	m_staticText2->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer2->Add( m_staticText2, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText4 = new wxStaticText( m_pTasks, wxID_ANY, wxT("Log level"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer2->Add( m_staticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxString m_choice1Choices[] = { wxT("Trace"), wxT("Info"), wxT("Warnings"), wxT("Errors"), wxT("Fatal") };
	int m_choice1NChoices = sizeof( m_choice1Choices ) / sizeof( wxString );
	m_choice1 = new wxChoice( m_pTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice1NChoices, m_choice1Choices, 0 );
	m_choice1->SetSelection( 0 );
	m_choice1->SetMinSize( wxSize( 150,-1 ) );

	bSizer2->Add( m_choice1, 0, wxALL, 5 );

	fgSizer1->Add( bSizer2, 1, wxEXPAND, 5 );

	m_listCtrl2 = new wxListCtrl( m_pTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_NO_SORT_HEADER|wxLC_REPORT );
	m_listCtrl2->SetMinSize( wxSize( 400,200 ) );

	fgSizer1->Add( m_listCtrl2, 1, wxBOTTOM|wxEXPAND|wxLEFT, 5 );

	m_richText1 = new wxRichTextCtrl( m_pTasks, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0|wxHSCROLL|wxVSCROLL|wxWANTS_CHARS );
	fgSizer1->Add( m_richText1, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );

	m_staticText3 = new wxStaticText( m_pTasks, wxID_ANY, wxT("Task List"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	m_staticText3->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer5->Add( m_staticText3, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSizer5->Add( 0, 0, 1, wxEXPAND, 5 );

	m_bpButton5 = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( start ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpButton5->Enable( false );

	m_bpButton5->Enable( false );

	bSizer5->Add( m_bpButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_bpButton11 = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( btnAdd ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpButton11->Enable( false );

	m_bpButton11->Enable( false );

	bSizer5->Add( m_bpButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_bpButton21 = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( btnDel ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpButton21->Enable( false );

	m_bpButton21->Enable( false );

	bSizer5->Add( m_bpButton21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	fgSizer1->Add( bSizer5, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );

	fgSizer1->Add( bSizer3, 1, wxEXPAND, 5 );

	m_listCtrl21 = new wxListCtrl( m_pTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	fgSizer1->Add( m_listCtrl21, 0, wxBOTTOM|wxEXPAND|wxLEFT, 5 );

	m_pTasks->SetSizer( fgSizer1 );
	m_pTasks->Layout();
	fgSizer1->Fit( m_pTasks );
	m_mainnb->AddPage( m_pTasks, wxT("Tasks"), true, wxBitmap( panTasks ) );
	m_pReports = new wxPanel( m_mainnb, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mainnb->AddPage( m_pReports, wxT("Reports"), false, wxBitmap( panReports ) );
	m_pSettings = new wxPanel( m_mainnb, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mainnb->AddPage( m_pSettings, wxT("Settings"), false, wxBitmap( panSettings ) );

	bSizer1->Add( m_mainnb, 1, wxEXPAND | wxALL, 0 );

	this->SetSizer( bSizer1 );
    this->Layout();
	m_statusBar1 = new wiStatBar( this );
	SetStatusBar(m_statusBar1);
}

MainForm::~MainForm()
{
}

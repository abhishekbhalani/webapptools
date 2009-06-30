///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wiStatBar.h"

#include "wiGuiData.h"

#include "../images/btnAdd.xpm"
#include "../images/btnApply16.xpm"
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

	m_stActiveTasks = new wxStaticText( m_pTasks, wxID_ANY, _("Active tasks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stActiveTasks->Wrap( -1 );
	m_stActiveTasks->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer4->Add( m_stActiveTasks, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );

	m_bpStartTask = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( start_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpStartTask->Enable( false );

	m_bpStartTask->Enable( false );

	bSizer4->Add( m_bpStartTask, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_bpPauseTask = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( pause_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpPauseTask->Enable( false );

	m_bpPauseTask->Enable( false );

	bSizer4->Add( m_bpPauseTask, 0, wxALIGN_CENTER_VERTICAL, 0 );

	m_bpCancelTask = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( btnDel_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpCancelTask->Enable( false );

	m_bpCancelTask->Enable( false );

	bSizer4->Add( m_bpCancelTask, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	fgSizer1->Add( bSizer4, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_stTaskLog = new wxStaticText( m_pTasks, wxID_ANY, _("Task execution log"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stTaskLog->Wrap( -1 );
	m_stTaskLog->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer2->Add( m_stTaskLog, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	m_stLogLevel = new wxStaticText( m_pTasks, wxID_ANY, _("Log level"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stLogLevel->Wrap( -1 );
	bSizer2->Add( m_stLogLevel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxString m_chLogLevelChoices[] = { _("Trace"), _("Info"), _("Warnings"), _("Errors"), _("Fatal") };
	int m_chLogLevelNChoices = sizeof( m_chLogLevelChoices ) / sizeof( wxString );
	m_chLogLevel = new wxChoice( m_pTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chLogLevelNChoices, m_chLogLevelChoices, 0 );
	m_chLogLevel->SetSelection( 0 );
	m_chLogLevel->SetMinSize( wxSize( 150,-1 ) );

	bSizer2->Add( m_chLogLevel, 0, wxALL, 5 );

	fgSizer1->Add( bSizer2, 1, wxEXPAND, 5 );

	m_lstActiveTask = new wxListCtrl( m_pTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_NO_SORT_HEADER|wxLC_REPORT );
	m_lstActiveTask->SetMinSize( wxSize( 400,200 ) );

	fgSizer1->Add( m_lstActiveTask, 1, wxBOTTOM|wxEXPAND|wxLEFT, 5 );

	m_rtTask = new wxRichTextCtrl( m_pTasks, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_AUTO_URL|wxTE_READONLY|wxHSCROLL|wxVSCROLL|wxWANTS_CHARS );
	fgSizer1->Add( m_rtTask, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );

	m_stTaskList = new wxStaticText( m_pTasks, wxID_ANY, _("Task List"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stTaskList->Wrap( -1 );
	m_stTaskList->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer5->Add( m_stTaskList, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSizer5->Add( 0, 0, 1, wxEXPAND, 5 );

	m_bpTaskGo = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( start_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpTaskGo->Enable( false );

	m_bpTaskGo->Enable( false );

	bSizer5->Add( m_bpTaskGo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_bpTaskNew = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( btnAdd_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpTaskNew->Enable( false );

	m_bpTaskNew->Enable( false );

	bSizer5->Add( m_bpTaskNew, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_bpTaskDel = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( btnDel_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpTaskDel->Enable( false );

	m_bpTaskDel->Enable( false );

	bSizer5->Add( m_bpTaskDel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	fgSizer1->Add( bSizer5, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );

	fgSizer1->Add( bSizer3, 1, wxEXPAND, 5 );

	m_lstTaskList = new wxListCtrl( m_pTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
	fgSizer1->Add( m_lstTaskList, 0, wxBOTTOM|wxEXPAND|wxLEFT, 5 );

	m_pTasks->SetSizer( fgSizer1 );
	m_pTasks->Layout();
	fgSizer1->Fit( m_pTasks );
	m_mainnb->AddPage( m_pTasks, _("Tasks"), false, wxBitmap( panTasks_xpm ) );
	m_pReports = new wxPanel( m_mainnb, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mainnb->AddPage( m_pReports, _("Reports"), false, wxBitmap( panReports_xpm ) );
	m_pSettings = new wxPanel( m_mainnb, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_stServer = new wxStaticText( m_pSettings, wxID_ANY, _("Scanner address:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stServer->Wrap( -1 );
	fgSizer2->Add( m_stServer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );

	wxString m_chServersChoices[] = { _("hostname[:port]") };
	int m_chServersNChoices = sizeof( m_chServersChoices ) / sizeof( wxString );
	m_chServers = new wxChoice( m_pSettings, wxID_ANY, wxDefaultPosition, wxSize( 250,-1 ), m_chServersNChoices, m_chServersChoices, 0 );
	m_chServers->SetSelection( 0 );
	bSizer6->Add( m_chServers, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_bpConnect = new wxBitmapButton( m_pSettings, wxID_ANY, wxBitmap( start_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpConnect->SetToolTip( _("Connect") );

	m_bpConnect->SetToolTip( _("Connect") );

	bSizer6->Add( m_bpConnect, 0, wxBOTTOM|wxTOP, 5 );

	m_bpServerNew = new wxBitmapButton( m_pSettings, wxID_ANY, wxBitmap( btnAdd_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpServerNew->SetToolTip( _("Add new connection") );

	m_bpServerNew->SetToolTip( _("Add new connection") );

	bSizer6->Add( m_bpServerNew, 0, wxBOTTOM|wxTOP, 5 );

	m_bpServerDel = new wxBitmapButton( m_pSettings, wxID_ANY, wxBitmap( btnDel_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpServerDel->SetToolTip( _("Delete connection") );

	m_bpServerDel->SetToolTip( _("Delete connection") );

	bSizer6->Add( m_bpServerDel, 0, wxBOTTOM|wxTOP, 5 );

	fgSizer2->Add( bSizer6, 1, wxEXPAND, 5 );

	m_stLang = new wxStaticText( m_pSettings, wxID_ANY, _("Interface Language:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stLang->Wrap( -1 );
	fgSizer2->Add( m_stLang, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );

	wxString m_chLangsChoices[] = { _("English (default)") };
	int m_chLangsNChoices = sizeof( m_chLangsChoices ) / sizeof( wxString );
	m_chLangs = new wxChoice( m_pSettings, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chLangsNChoices, m_chLangsChoices, 0 );
	m_chLangs->SetSelection( 0 );
	m_chLangs->SetMinSize( wxSize( 250,-1 ) );

	bSizer7->Add( m_chLangs, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_bpLangApply = new wxBitmapButton( m_pSettings, wxID_ANY, wxBitmap( btnApply16_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpLangApply->SetToolTip( _("Apply (need restart)") );

	m_bpLangApply->SetToolTip( _("Apply (need restart)") );

	bSizer7->Add( m_bpLangApply, 0, wxBOTTOM|wxTOP, 5 );

	fgSizer2->Add( bSizer7, 1, wxEXPAND, 5 );

	m_pSettings->SetSizer( fgSizer2 );
	m_pSettings->Layout();
	fgSizer2->Fit( m_pSettings );
	m_mainnb->AddPage( m_pSettings, _("Settings"), true, wxBitmap( panSettings_xpm ) );

	bSizer1->Add( m_mainnb, 1, wxEXPAND | wxALL, 0 );

	this->SetSizer( bSizer1 );
	this->Layout();
    m_statusBar1 = new wiStatBar( this );
    SetStatusBar(m_statusBar1);

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MainForm::OnClose ) );
	m_chLangs->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MainForm::OnLangChange ), NULL, this );
}

MainForm::~MainForm()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MainForm::OnClose ) );
	m_chLangs->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MainForm::OnLangChange ), NULL, this );
}

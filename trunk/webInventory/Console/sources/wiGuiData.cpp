///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wiStatBar.h"
#include "wxThings.h"

#include "wiGuiData.h"

#include "../images/apply.xpm"
#include "../images/btnAdd.xpm"
#include "../images/btnApply16.xpm"
#include "../images/btnDel.xpm"
#include "../images/btnEdit.xpm"
#include "../images/btnStop.xpm"
#include "../images/panReports.xpm"
#include "../images/panSettings.xpm"
#include "../images/panTasks.xpm"
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
	fgSizer1 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->AddGrowableRow( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );

	m_stTaskList = new wxStaticText( m_pTasks, wxID_ANY, _("Task List"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stTaskList->Wrap( -1 );
	m_stTaskList->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer4->Add( m_stTaskList, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSizer4->Add( 0, 0, 10, wxEXPAND, 5 );

	m_bpTaskGo = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( start_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpTaskGo->Enable( false );
	m_bpTaskGo->SetToolTip( _("Start task") );

	m_bpTaskGo->Enable( false );
	m_bpTaskGo->SetToolTip( _("Start task") );

	bSizer4->Add( m_bpTaskGo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_bpCancelTask = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( btnStop_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpCancelTask->Enable( false );
	m_bpCancelTask->SetToolTip( _("Cancel task") );

	m_bpCancelTask->Enable( false );
	m_bpCancelTask->SetToolTip( _("Cancel task") );

	bSizer4->Add( m_bpCancelTask, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );


	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );

	m_bpTaskNew = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( btnAdd_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpTaskNew->Enable( false );
	m_bpTaskNew->SetToolTip( _("Add new task") );

	m_bpTaskNew->Enable( false );
	m_bpTaskNew->SetToolTip( _("Add new task") );

	bSizer4->Add( m_bpTaskNew, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	m_bpTaskDel = new wxBitmapButton( m_pTasks, wxID_ANY, wxBitmap( btnDel_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpTaskDel->Enable( false );
	m_bpTaskDel->SetToolTip( _("Delete task") );

	m_bpTaskDel->Enable( false );
	m_bpTaskDel->SetToolTip( _("Delete task") );

	bSizer4->Add( m_bpTaskDel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	bSizer121->Add( bSizer4, 0, wxEXPAND, 5 );

	m_lstTaskList = new wxListCtrl( m_pTasks, wxID_ANY, wxDefaultPosition, wxSize( 300,-1 ), wxLC_REPORT );
	bSizer121->Add( m_lstTaskList, 1, wxBOTTOM|wxLEFT, 5 );

	fgSizer1->Add( bSizer121, 1, wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer6->AddGrowableCol( 0 );
	fgSizer6->AddGrowableRow( 3 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_panTaskOpts = new wxPanel( m_pTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panTaskOpts->Enable( false );

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );

	m_stProfile = new wxStaticText( m_panTaskOpts, wxID_ANY, _("Task settings"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stProfile->Wrap( -1 );
	m_stProfile->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSizer11->Add( m_stProfile, 0, wxALIGN_BOTTOM|wxALL, 5 );

	wxGridBagSizer* gbSizer2;
	gbSizer2 = new wxGridBagSizer( 0, 0 );
	gbSizer2->AddGrowableCol( 3 );
	gbSizer2->AddGrowableRow( 10 );
	gbSizer2->SetFlexibleDirection( wxBOTH );
	gbSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_stTaskName = new wxStaticText( m_panTaskOpts, wxID_ANY, _("Task name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stTaskName->Wrap( -1 );
	gbSizer2->Add( m_stTaskName, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_txtTaskName = new wxTextCtrl( m_panTaskOpts, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_txtTaskName->SetMinSize( wxSize( 200,-1 ) );

	gbSizer2->Add( m_txtTaskName, wxGBPosition( 0, 1 ), wxGBSpan( 1, 2 ), wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );

	m_stBaseURL = new wxStaticText( m_panTaskOpts, wxID_ANY, _("Base URL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stBaseURL->Wrap( -1 );
	gbSizer2->Add( m_stBaseURL, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_txtBaseURL = new wxTextCtrl( m_panTaskOpts, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_txtBaseURL->SetMinSize( wxSize( 200,-1 ) );

	gbSizer2->Add( m_txtBaseURL, wxGBPosition( 1, 1 ), wxGBSpan( 1, 2 ), wxALL|wxEXPAND, 5 );

	wxString m_rbDepthChoices[] = { _("Stay in dir"), _("Stay in host"), _("Stay in domain") };
	int m_rbDepthNChoices = sizeof( m_rbDepthChoices ) / sizeof( wxString );
	m_rbDepth = new wxRadioBox( m_panTaskOpts, wxID_ANY, _("Depth mode"), wxDefaultPosition, wxDefaultSize, m_rbDepthNChoices, m_rbDepthChoices, 1, wxRA_SPECIFY_COLS );
	m_rbDepth->SetSelection( 0 );
	gbSizer2->Add( m_rbDepth, wxGBPosition( 2, 0 ), wxGBSpan( 3, 2 ), wxALL|wxEXPAND, 5 );

	m_stDepth = new wxStaticText( m_panTaskOpts, wxID_ANY, _("Scan depth"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stDepth->Wrap( -1 );
	gbSizer2->Add( m_stDepth, wxGBPosition( 2, 2 ), wxGBSpan( 1, 1 ), wxALIGN_BOTTOM|wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	m_textCtrl7 = new wxTextCtrl( m_panTaskOpts, wxID_ANY, _("-1"), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer2->Add( m_textCtrl7, wxGBPosition( 3, 2 ), wxGBSpan( 1, 1 ), wxALIGN_TOP|wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	m_btnApply = new wxCustomButton( m_panTaskOpts, wxID_ANY, _("Apply"), wxBitmap( apply_xpm ), wxDefaultPosition, wxDefaultSize, wxCUSTBUT_BUTTON|wxCUSTBUT_RIGHT);
	gbSizer2->Add( m_btnApply, wxGBPosition( 0, 3 ), wxGBSpan( 2, 1 ), wxALIGN_RIGHT|wxALL, 5 );

	m_cbInvent = new wxCheckBox( m_panTaskOpts, wxID_ANY, _("Inventory only"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbInvent->SetValue(true);

	m_cbInvent->Enable( false );

	gbSizer2->Add( m_cbInvent, wxGBPosition( 4, 2 ), wxGBSpan( 1, 1 ), wxALL, 5 );

	bSizer11->Add( gbSizer2, 1, wxEXPAND, 0 );

	m_panTaskOpts->SetSizer( bSizer11 );
	m_panTaskOpts->Layout();
	bSizer11->Fit( m_panTaskOpts );
	fgSizer6->Add( m_panTaskOpts, 1, wxEXPAND | wxALL, 5 );

	m_staticline1 = new wxStaticLine( m_pTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	fgSizer6->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

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

	fgSizer6->Add( bSizer2, 0, wxEXPAND, 5 );

	m_rtTask = new wxRichTextCtrl( m_pTasks, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxTE_AUTO_URL|wxTE_READONLY|wxHSCROLL|wxVSCROLL|wxWANTS_CHARS );
	fgSizer6->Add( m_rtTask, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5 );

	fgSizer1->Add( fgSizer6, 1, wxEXPAND, 5 );

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

	m_stConVers = new wxStaticText( m_pSettings, wxID_ANY, _("Console version:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stConVers->Wrap( -1 );
	fgSizer2->Add( m_stConVers, 0, wxALL, 5 );

	m_stConVersData = new wxStaticText( m_pSettings, wxID_ANY, _("unknown"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stConVersData->Wrap( -1 );
	fgSizer2->Add( m_stConVersData, 0, wxALL, 5 );

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

	m_bpSrvEdit = new wxBitmapButton( m_pSettings, wxID_ANY, wxBitmap( btnEdit_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpSrvEdit->SetToolTip( _("Edit connection") );

	m_bpSrvEdit->SetToolTip( _("Edit connection") );

	bSizer6->Add( m_bpSrvEdit, 0, wxBOTTOM|wxTOP, 5 );

	m_bpServerDel = new wxBitmapButton( m_pSettings, wxID_ANY, wxBitmap( btnDel_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpServerDel->SetToolTip( _("Delete connection") );

	m_bpServerDel->SetToolTip( _("Delete connection") );

	bSizer6->Add( m_bpServerDel, 0, wxBOTTOM|wxTOP, 5 );

	fgSizer2->Add( bSizer6, 1, wxEXPAND, 5 );

	m_stSrvVers = new wxStaticText( m_pSettings, wxID_ANY, _("Scanner version:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stSrvVers->Wrap( -1 );
	fgSizer2->Add( m_stSrvVers, 0, wxALL, 5 );

	m_stSrvVersData = new wxStaticText( m_pSettings, wxID_ANY, _("unknown"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stSrvVersData->Wrap( -1 );
	fgSizer2->Add( m_stSrvVersData, 0, wxALL, 5 );

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

	m_stLangRestart = new wxStaticText( m_pSettings, wxID_ANY, _("Need to restart to apply changes"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stLangRestart->Wrap( -1 );
	m_stLangRestart->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );
	m_stLangRestart->SetForegroundColour( wxColour( 255, 70, 70 ) );
	m_stLangRestart->Hide();

	bSizer7->Add( m_stLangRestart, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	fgSizer2->Add( bSizer7, 1, wxEXPAND, 5 );

	m_pSettings->SetSizer( fgSizer2 );
	m_pSettings->Layout();
	fgSizer2->Fit( m_pSettings );
	m_mainnb->AddPage( m_pSettings, _("Settings"), true, wxBitmap( panSettings_xpm ) );

	bSizer1->Add( m_mainnb, 1, wxEXPAND | wxALL, 0 );

	this->SetSizer( bSizer1 );
	this->Layout();
    m_statusBar = new wiStatBar( this );
    SetStatusBar(m_statusBar);

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MainForm::OnClose ) );
	m_bpTaskGo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnRunTask ), NULL, this );
	m_bpCancelTask->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnCancelTask ), NULL, this );
	m_bpTaskNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnAddTask ), NULL, this );
	m_bpTaskDel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnDelTask ), NULL, this );
	m_lstTaskList->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( MainForm::OnSortItems ), NULL, this );
	m_lstTaskList->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( MainForm::OnTaskSelected ), NULL, this );
	m_bpConnect->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnConnect ), NULL, this );
	m_bpServerNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnAddServer ), NULL, this );
	m_bpSrvEdit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnEditServer ), NULL, this );
	m_bpServerDel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnDelServer ), NULL, this );
	m_bpLangApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnLangChange ), NULL, this );
}

MainForm::~MainForm()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MainForm::OnClose ) );
	m_bpTaskGo->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnRunTask ), NULL, this );
	m_bpCancelTask->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnCancelTask ), NULL, this );
	m_bpTaskNew->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnAddTask ), NULL, this );
	m_bpTaskDel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnDelTask ), NULL, this );
	m_lstTaskList->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( MainForm::OnSortItems ), NULL, this );
	m_lstTaskList->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( MainForm::OnTaskSelected ), NULL, this );
	m_bpConnect->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnConnect ), NULL, this );
	m_bpServerNew->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnAddServer ), NULL, this );
	m_bpSrvEdit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnEditServer ), NULL, this );
	m_bpServerDel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnDelServer ), NULL, this );
	m_bpLangApply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnLangChange ), NULL, this );
}

ServDialog::ServDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_stName = new wxStaticText( this, wxID_ANY, _("Scanner name:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stName->Wrap( -1 );
	fgSizer3->Add( m_stName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_txtName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_txtName->SetMinSize( wxSize( 200,-1 ) );

	fgSizer3->Add( m_txtName, 0, wxALL, 5 );

	m_stServName = new wxStaticText( this, wxID_ANY, _("Scanner host:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stServName->Wrap( -1 );
	fgSizer3->Add( m_stServName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_txtHostname = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_txtHostname->SetMinSize( wxSize( 200,-1 ) );

	fgSizer3->Add( m_txtHostname, 0, wxALL, 5 );

	m_stPort = new wxStaticText( this, wxID_ANY, _("Scanner port:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stPort->Wrap( -1 );
	fgSizer3->Add( m_stPort, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_txtSrvPort = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_txtSrvPort->SetMinSize( wxSize( 200,-1 ) );

	fgSizer3->Add( m_txtSrvPort, 0, wxALL, 5 );

	bSizer8->Add( fgSizer3, 1, wxEXPAND, 5 );

	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	bSizer8->Add( m_sdbSizer1, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	m_sdbSizer1OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ServDialog::OnOK ), NULL, this );
}

ServDialog::~ServDialog()
{
	// Disconnect Events
	m_sdbSizer1OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ServDialog::OnOK ), NULL, this );
}

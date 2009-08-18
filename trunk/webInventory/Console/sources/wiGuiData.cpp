///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wiStatBar.h"

#include "wiGuiData.h"

#include "../images/_btnAdd.xpm"
#include "../images/_btnApply16.xpm"
#include "../images/_btnDel.xpm"

#include "../images/btnAdd.xpm"
#include "../images/btnApply16.xpm"
#include "../images/btnDel.xpm"
#include "../images/btnEdit.xpm"
#include "../images/btnStop.xpm"
#include "../images/date.xpm"
#include "../images/filter.xpm"
#include "../images/flsave.xpm"
#include "../images/flstatus.xpm"
#include "../images/opts_audit.xpm"
#include "../images/panReports.xpm"
#include "../images/panSettings.xpm"
#include "../images/panTasks.xpm"
#include "../images/reload.xpm"
#include "../images/start.xpm"
#include "../images/tree_no.xpm"

///////////////////////////////////////////////////////////////////////////

MainForm::MainForm( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 800,600 ), wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	m_mainnb = new wxAuiNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_SCROLL_BUTTONS|wxAUI_NB_TAB_FIXED_WIDTH );
	m_pTasks = new wxPanel( m_mainnb, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSzTab1;
	bSzTab1 = new wxBoxSizer( wxVERTICAL );

	m_splitTasks = new wxSplitterWindow( m_pTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH );
	m_splitTasks->Connect( wxEVT_IDLE, wxIdleEventHandler( MainForm::m_splitTasksOnIdle ), NULL, this );
	m_panObjects = new wxPanel( m_splitTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSzSplitObjects;
	bSzSplitObjects = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSzObjectsList;
	bSzObjectsList = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSzObjTools;
	bSzObjTools = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText* m_stTaskList;
	m_stTaskList = new wxStaticText( m_panObjects, wxID_ANY, _("Targets List"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stTaskList->Wrap( -1 );
	m_stTaskList->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSzObjTools->Add( m_stTaskList, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSzObjTools->Add( 0, 0, 1, wxEXPAND, 5 );
	wxInitAllImageHandlers();

	m_toolBarObject = new wxToolBar( m_panObjects, wxID_TBOBJ, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER );
	m_toolBarObject->SetToolBitmapSize( wxSize( 16,16 ) );
	m_toolBarObject->AddTool( wxID_TOOLNEW, wxEmptyString, wxBitmap( btnAdd_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Add new object"), wxEmptyString );
	m_toolBarObject->AddTool( wxID_TOOLEDIT, wxEmptyString, wxBitmap( btnEdit_xpm ), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString );
	m_toolBarObject->AddTool( wxID_TOOLDEL, wxEmptyString, wxBitmap( btnDel_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Delete task"), wxEmptyString );
	m_toolBarObject->Realize();

	bSzObjTools->Add( m_toolBarObject, 0, wxEXPAND, 0 );

	bSzObjectsList->Add( bSzObjTools, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );

	m_lstObjectList = new wxListCtrl( m_panObjects, wxID_ANY, wxDefaultPosition, wxSize( 200,-1 ), wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	bSzObjectsList->Add( m_lstObjectList, 1, wxLEFT|wxRIGHT, 5 );

	bSzSplitObjects->Add( bSzObjectsList, 0, wxEXPAND, 0 );

	wxBoxSizer* bSzProfile;
	bSzProfile = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSzProfileTool;
	bSzProfileTool = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText* m_stProfile;
	m_stProfile = new wxStaticText( m_panObjects, wxID_ANY, _("Profile settings"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stProfile->Wrap( -1 );
	m_stProfile->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSzProfileTool->Add( m_stProfile, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSzProfileTool->Add( 0, 0, 1, wxEXPAND, 5 );

	m_toolBarProf = new wxToolBar( m_panObjects, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER );
	m_toolBarProf->SetToolBitmapSize( wxSize( 16,16 ) );
	wxArrayString m_chProfileChoices;
	m_chProfile = new wxChoice( m_toolBarProf, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chProfileChoices, 0 );
	m_chProfile->SetSelection( 0 );
	m_toolBarProf->AddControl( m_chProfile );
	m_toolBarProf->AddTool( wxID_TLPROFNEW, _("tool"), wxBitmap( btnAdd_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Create new profile"), wxEmptyString );
	m_toolBarProf->AddTool( wxID_TLPROFCLONE, _("tool"), wxBitmap( reload_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Copy current profile"), wxEmptyString );
	m_toolBarProf->AddTool( wxID_TLPROFDEL, _("tool"), wxBitmap( btnDel_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Delete profile"), wxEmptyString );
	m_toolBarProf->AddSeparator();
	m_toolBarProf->AddTool( wxID_TLPROFSAVE, _("tool"), wxBitmap( flsave_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Save profile"), wxEmptyString );
	m_toolBarProf->AddSeparator();
	m_toolBarProf->AddTool( wxID_TOOLGO, wxEmptyString, wxBitmap( start_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Start task"), wxEmptyString );
	m_toolBarProf->Realize();

	bSzProfileTool->Add( m_toolBarProf, 0, wxEXPAND, 5 );

	bSzProfile->Add( bSzProfileTool, 0, wxEXPAND, 5 );

	wxBoxSizer* bSzProfileSettings;
	bSzProfileSettings = new wxBoxSizer( wxHORIZONTAL );

	m_bpAddPlugin = new wxBitmapButton( m_panObjects, wxID_ANY, wxBitmap( btnAdd_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );

	m_bpAddPlugin->SetBitmapDisabled( wxBitmap( _btnAdd_xpm ) );
	m_bpAddPlugin->SetToolTip( _("Add plugin") );

	bSzProfileSettings->Add( m_bpAddPlugin, 0, wxBOTTOM|wxLEFT|wxTOP, 5 );

	m_bpDelPlugin = new wxBitmapButton( m_panObjects, wxID_ANY, wxBitmap( btnDel_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );

	m_bpDelPlugin->SetBitmapDisabled( wxBitmap( _btnDel_xpm ) );
	m_bpDelPlugin->SetToolTip( _("Remove plugin") );

	bSzProfileSettings->Add( m_bpDelPlugin, 0, wxBOTTOM|wxRIGHT|wxTOP, 5 );

	wxStaticText* m_staticText23;
	m_staticText23 = new wxStaticText( m_panObjects, wxID_ANY, _("Profile name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText23->Wrap( -1 );
	bSzProfileSettings->Add( m_staticText23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_txtProfName = new wxTextCtrl( m_panObjects, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, wxT("name<8>[0]") );
	bSzProfileSettings->Add( m_txtProfName, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	bSzProfile->Add( bSzProfileSettings, 0, wxEXPAND, 0 );

	m_plgBook = new wxTreebook( m_panObjects, wxID_ANY, wxDefaultPosition, wxDefaultSize );
	wxSize m_plgBookImageSize = wxSize( 16,16 );
	int m_plgBookIndex = 0;
	wxImageList* m_plgBookImages = new wxImageList( m_plgBookImageSize.GetWidth(), m_plgBookImageSize.GetHeight() );
	m_plgBook->AssignImageList( m_plgBookImages );
	wxBitmap m_plgBookBitmap;
	wxImage m_plgBookImage;
	m_panTaskOpts = new wxScrolledWindow( m_plgBook, wxID_ANY, wxDefaultPosition, wxSize( -1,200 ), wxHSCROLL|wxVSCROLL );
	m_panTaskOpts->SetScrollRate( 5, 5 );
	m_panTaskOpts->SetMinSize( wxSize( -1,200 ) );

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );

	wxGridBagSizer* gbSizer2;
	gbSizer2 = new wxGridBagSizer( 0, 0 );
	gbSizer2->AddGrowableCol( 4 );
	gbSizer2->AddGrowableRow( 10 );
	gbSizer2->SetFlexibleDirection( wxBOTH );
	gbSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_stLogLevel;
	m_stLogLevel = new wxStaticText( m_panTaskOpts, wxID_ANY, _("Log level"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stLogLevel->Wrap( -1 );
	gbSizer2->Add( m_stLogLevel, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxString m_chLogLevelChoices[] = { _("Trace"), _("Debug"), _("Info"), _("Warnings"), _("Errors"), _("Fatal") };
	int m_chLogLevelNChoices = sizeof( m_chLogLevelChoices ) / sizeof( wxString );
	m_chLogLevel = new wxChoice( m_panTaskOpts, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chLogLevelNChoices, m_chLogLevelChoices, 0, wxDefaultValidator, wxT("LogLevel<2>") );
	m_chLogLevel->SetSelection( 0 );
	m_chLogLevel->SetMinSize( wxSize( 150,-1 ) );

	gbSizer2->Add( m_chLogLevel, wxGBPosition( 0, 1 ), wxGBSpan( 1, 2 ), wxALL|wxEXPAND, 5 );


	gbSizer2->Add( 0, 0, wxGBPosition( 3, 4 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );

	bSizer11->Add( gbSizer2, 0, wxEXPAND, 0 );

	m_panTaskOpts->SetSizer( bSizer11 );
	m_panTaskOpts->Layout();
	m_plgBook->AddPage( m_panTaskOpts, _("Generic options"), false );

	bSzProfile->Add( m_plgBook, 1, wxEXPAND | wxALL, 0 );

	bSzSplitObjects->Add( bSzProfile, 1, wxEXPAND, 5 );

	m_panObjects->SetSizer( bSzSplitObjects );
	m_panObjects->Layout();
	bSzSplitObjects->Fit( m_panObjects );
	m_panTasks = new wxPanel( m_splitTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSzTasks;
	bSzTasks = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSzTaskTools;
	bSzTaskTools = new wxBoxSizer( wxHORIZONTAL );

	wxStaticText* m_stTaskLog;
	m_stTaskLog = new wxStaticText( m_panTasks, wxID_ANY, _("Active tasks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stTaskLog->Wrap( -1 );
	m_stTaskLog->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	bSzTaskTools->Add( m_stTaskLog, 0, wxALIGN_BOTTOM|wxALL, 5 );


	bSzTaskTools->Add( 20, 0, 0, 0, 5 );

	m_toolBarTasks = new wxToolBar( m_panTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER );
	m_toolBarTasks->SetToolBitmapSize( wxSize( 16,16 ) );
	m_toolBarTasks->AddTool( wxID_TOOLPAUSE, wxEmptyString, wxBitmap( start_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Start task"), wxEmptyString );
	m_toolBarTasks->AddTool( wxID_TOOLSTOP, wxEmptyString, wxBitmap( btnStop_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Cancel task"), wxEmptyString );
	m_toolBarTasks->Realize();

	bSzTaskTools->Add( m_toolBarTasks, 0, wxEXPAND, 5 );

	bSzTasks->Add( bSzTaskTools, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );

	wxBoxSizer* bSzTaskInfo;
	bSzTaskInfo = new wxBoxSizer( wxHORIZONTAL );

	m_splitLogs = new wxSplitterWindow( m_panTasks, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_splitLogs->Connect( wxEVT_IDLE, wxIdleEventHandler( MainForm::m_splitLogsOnIdle ), NULL, this );
	wxPanel* m_panTaskList;
	m_panTaskList = new wxPanel( m_splitLogs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );

	m_lstTaskList = new wxListCtrl( m_panTaskList, wxID_ANY, wxDefaultPosition, wxSize( 300,-1 ), wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL );
	bSizer24->Add( m_lstTaskList, 1, wxEXPAND|wxLEFT|wxRIGHT, 0 );

	m_panTaskList->SetSizer( bSizer24 );
	m_panTaskList->Layout();
	bSizer24->Fit( m_panTaskList );
	wxPanel* m_paTaskLog;
	m_paTaskLog = new wxPanel( m_splitLogs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );

	m_rtTask = new wxRichTextCtrl( m_paTaskLog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxTE_AUTO_URL|wxTE_READONLY|wxHSCROLL|wxVSCROLL|wxWANTS_CHARS );
	bSizer25->Add( m_rtTask, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 0 );

	m_paTaskLog->SetSizer( bSizer25 );
	m_paTaskLog->Layout();
	bSizer25->Fit( m_paTaskLog );
	m_splitLogs->SplitVertically( m_panTaskList, m_paTaskLog, 300 );
	bSzTaskInfo->Add( m_splitLogs, 1, wxEXPAND, 0 );

	bSzTasks->Add( bSzTaskInfo, 1, wxEXPAND, 5 );

	m_panTasks->SetSizer( bSzTasks );
	m_panTasks->Layout();
	bSzTasks->Fit( m_panTasks );
	m_splitTasks->SplitHorizontally( m_panObjects, m_panTasks, 0 );
	bSzTab1->Add( m_splitTasks, 1, wxEXPAND, 0 );

	m_pTasks->SetSizer( bSzTab1 );
	m_pTasks->Layout();
	bSzTab1->Fit( m_pTasks );
	m_mainnb->AddPage( m_pTasks, _("Tasks"), false, wxBitmap( panTasks_xpm ) );
	m_pReports = new wxPanel( m_mainnb, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );

	m_split = new wxSplitterWindow( m_pReports, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_split->SetSashGravity( 0.66 );
	m_split->Connect( wxEVT_IDLE, wxIdleEventHandler( MainForm::m_splitOnIdle ), NULL, this );
	m_panRepTree = new wxPanel( m_split, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );

	m_toolBarFilter = new wxToolBar( m_panRepTree, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER );
	m_toolBarFilter->SetToolBitmapSize( wxSize( 16,16 ) );
	m_toolBarFilter->AddTool( wxID_TLREFRESH, _("tool"), wxBitmap( reload_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Refresh reports list"), wxEmptyString );
	m_toolBarFilter->AddTool( wxID_TLFILTER, _("tool"), wxBitmap( filter_xpm ), wxNullBitmap, wxITEM_CHECK, _("Apply filter"), wxEmptyString );
	wxStaticText* m_stToolObject;
	m_stToolObject = new wxStaticText( m_toolBarFilter, wxID_ANY, _("Object: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_stToolObject->Wrap( -1 );
	m_toolBarFilter->AddControl( m_stToolObject );
	wxArrayString m_chTaskFilterChoices;
	m_chTaskFilter = new wxChoice( m_toolBarFilter, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chTaskFilterChoices, 0 );
	m_chTaskFilter->SetSelection( 0 );
	m_toolBarFilter->AddControl( m_chTaskFilter );
	m_toolBarFilter->AddTool( wxID_TLDATE, _("tool"), wxBitmap( date_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Date filter mode"), wxEmptyString );
	m_dateFilter = new wxDatePickerCtrl( m_toolBarFilter, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT|wxDP_DROPDOWN|wxDP_SHOWCENTURY );
	m_toolBarFilter->AddControl( m_dateFilter );
	m_toolBarFilter->AddTool( wxID_TLSTATUS, _("tool"), wxBitmap( flstatus_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Filter by scan status"), wxEmptyString );
	m_toolBarFilter->AddTool( wxID_TLSAVE, _("tool"), wxBitmap( flsave_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Save and load filters"), wxEmptyString );
	m_toolBarFilter->Realize();

	bSizer14->Add( m_toolBarFilter, 0, wxEXPAND, 5 );

	m_treeScans = new wxTreeCtrl( m_panRepTree, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_FULL_ROW_HIGHLIGHT|wxTR_HAS_BUTTONS|wxTR_HIDE_ROOT|wxTR_LINES_AT_ROOT|wxTR_SINGLE );
	bSizer14->Add( m_treeScans, 1, wxALL|wxEXPAND, 0 );

	m_panRepTree->SetSizer( bSizer14 );
	m_panRepTree->Layout();
	bSizer14->Fit( m_panRepTree );
	m_panRepData = new wxPanel( m_split, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxVERTICAL );

	m_richText2 = new wxRichTextCtrl( m_panRepData, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0|wxVSCROLL|wxHSCROLL|wxNO_BORDER|wxWANTS_CHARS );
	bSizer15->Add( m_richText2, 1, wxEXPAND | wxALL, 0 );

	m_panRepData->SetSizer( bSizer15 );
	m_panRepData->Layout();
	bSizer15->Fit( m_panRepData );
	m_split->SplitVertically( m_panRepTree, m_panRepData, 400 );
	bSizer13->Add( m_split, 1, wxEXPAND, 5 );

	m_pReports->SetSizer( bSizer13 );
	m_pReports->Layout();
	bSizer13->Fit( m_pReports );
	m_mainnb->AddPage( m_pReports, _("Reports"), false, wxBitmap( panReports_xpm ) );
	m_pSettings = new wxPanel( m_mainnb, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );

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

	m_toolBarSrv = new wxToolBar( m_pSettings, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER );
	m_toolBarSrv->SetToolBitmapSize( wxSize( 16,16 ) );
	m_toolBarSrv->AddTool( wxID_TLCONNECT, wxEmptyString, wxBitmap( start_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Connect"), wxEmptyString );
	m_toolBarSrv->AddTool( wxID_TLNEW, wxEmptyString, wxBitmap( btnAdd_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Add new connection"), wxEmptyString );
	m_toolBarSrv->AddTool( wxID_TLEDIT, _("tool"), wxBitmap( btnEdit_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Edit connection"), wxEmptyString );
	m_toolBarSrv->AddTool( wxID_TLDELETE, _("tool"), wxBitmap( btnDel_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Delete connection"), wxEmptyString );
	m_toolBarSrv->Realize();

	bSizer6->Add( m_toolBarSrv, 0, wxALIGN_CENTER_VERTICAL, 5 );

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

	m_toolBarLang = new wxToolBar( m_pSettings, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER );
	m_toolBarLang->SetToolBitmapSize( wxSize( 16,16 ) );
	m_toolBarLang->AddTool( wxID_TLLANGAPPLY, _("tool"), wxBitmap( btnApply16_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Apply (need restart)"), wxEmptyString );
	m_toolBarLang->Realize();

	bSizer7->Add( m_toolBarLang, 0, wxALIGN_CENTER_VERTICAL, 5 );

	m_stLangRestart = new wxStaticText( m_pSettings, wxID_ANY, _("Need to restart to apply changes"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stLangRestart->Wrap( -1 );
	m_stLangRestart->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );
	m_stLangRestart->SetForegroundColour( wxColour( 255, 70, 70 ) );
	m_stLangRestart->Hide();

	bSizer7->Add( m_stLangRestart, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	fgSizer2->Add( bSizer7, 1, wxEXPAND, 5 );

	wxStaticText* m_stLogging;
	m_stLogging = new wxStaticText( m_pSettings, wxID_ANY, _("Write console logs:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stLogging->Wrap( -1 );
	fgSizer2->Add( m_stLogging, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_chLogging = new wxCheckBox( m_pSettings, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );

	fgSizer2->Add( m_chLogging, 0, wxALL, 5 );


	fgSizer2->Add( 120, 0, 1, wxEXPAND, 5 );

	bSizer10->Add( fgSizer2, 0, wxEXPAND, 5 );

	m_staticline2 = new wxStaticLine( m_pSettings, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer10->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );

	m_pnServer = new wxPanel( m_pSettings, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer61;
	fgSizer61 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer61->SetFlexibleDirection( wxBOTH );
	fgSizer61->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText18 = new wxStaticText( m_pnServer, wxID_ANY, _("Scanner storage:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	fgSizer61->Add( m_staticText18, 0, wxALL, 5 );

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );

	wxArrayString m_chStorageChoices;
	m_chStorage = new wxChoice( m_pnServer, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), m_chStorageChoices, 0 );
	m_chStorage->SetSelection( 0 );
	m_chStorage->SetMinSize( wxSize( 250,-1 ) );

	bSizer12->Add( m_chStorage, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_bpStorageApply = new wxBitmapButton( m_pnServer, wxID_ANY, wxBitmap( btnApply16_xpm ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );

	m_bpStorageApply->SetBitmapDisabled( wxBitmap( _btnApply16_xpm ) );
	m_bpStorageApply->SetToolTip( _("Apply") );

	bSizer12->Add( m_bpStorageApply, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxTOP, 5 );


	bSizer12->Add( 0, 0, 1, wxEXPAND, 5 );

	m_tbServer = new wxToolBar( m_pnServer, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER );
	m_tbServer->SetToolBitmapSize( wxSize( 16,16 ) );
	m_tbServer->AddTool( wxID_TLPLGRELOAD, _("tool"), wxBitmap( reload_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Refresh plugins list"), wxEmptyString );
	m_tbServer->AddTool( wxID_TLSRVSTOP, _("tool"), wxBitmap( tree_no_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Stops the server"), wxEmptyString );
	m_tbServer->AddTool( wxID_TLSRVLOG, _("tool"), wxBitmap( opts_audit_xpm ), wxNullBitmap, wxITEM_NORMAL, _("View server log"), wxEmptyString );
	m_tbServer->Realize();

	bSizer12->Add( m_tbServer, 0, wxALIGN_CENTER_VERTICAL, 0 );

	fgSizer61->Add( bSizer12, 1, wxEXPAND, 5 );

	m_staticText19 = new wxStaticText( m_pnServer, wxID_ANY, _("Scanner plugins:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19->Wrap( -1 );
	fgSizer61->Add( m_staticText19, 0, wxALL, 5 );

	m_pluginsDock = new wxScrolledWindow( m_pnServer, wxID_ANY, wxDefaultPosition, wxSize( 520,300 ), wxHSCROLL|wxVSCROLL );
	m_pluginsDock->SetScrollRate( 5, 5 );
	m_pluginsDock->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_INFOBK ) );
	m_pluginsDock->SetMinSize( wxSize( 520,300 ) );

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	bSizer9->SetMinSize( wxSize( 500,300 ) );
	m_gbPluginsGrid = new wxGridBagSizer( 0, 0 );
	m_gbPluginsGrid->AddGrowableCol( 1 );
	m_gbPluginsGrid->SetFlexibleDirection( wxBOTH );
	m_gbPluginsGrid->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_gbPluginsGrid->SetMinSize( wxSize( 500,-1 ) );
	m_btPluginIcon = new wxButton( m_pluginsDock, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 24,-1 ), wxNO_BORDER );
	m_gbPluginsGrid->Add( m_btPluginIcon, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 0 );

	m_btPluginDesc = new wxButton( m_pluginsDock, wxID_ANY, _("Description"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
	m_gbPluginsGrid->Add( m_btPluginDesc, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 0 );

	m_btPluginIface = new wxButton( m_pluginsDock, wxID_ANY, _("Interface"), wxDefaultPosition, wxSize( 120,-1 ), wxNO_BORDER );
	m_gbPluginsGrid->Add( m_btPluginIface, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 0 );

	bSizer9->Add( m_gbPluginsGrid, 1, wxEXPAND, 5 );

	m_pluginsDock->SetSizer( bSizer9 );
	m_pluginsDock->Layout();
	fgSizer61->Add( m_pluginsDock, 1, wxEXPAND|wxLEFT, 5 );


	fgSizer61->Add( 120, 0, 1, wxEXPAND, 5 );

	m_pnServer->SetSizer( fgSizer61 );
	m_pnServer->Layout();
	fgSizer61->Fit( m_pnServer );
	bSizer10->Add( m_pnServer, 1, wxEXPAND | wxALL, 0 );

	m_pSettings->SetSizer( bSizer10 );
	m_pSettings->Layout();
	bSizer10->Fit( m_pSettings );
	m_mainnb->AddPage( m_pSettings, _("Settings"), true, wxBitmap( panSettings_xpm ) );

	bSizer1->Add( m_mainnb, 1, wxEXPAND | wxALL, 0 );

	this->SetSizer( bSizer1 );
	this->Layout();
	m_statusBar = new wiStatBar( this );
	SetStatusBar(m_statusBar);

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MainForm::OnClose ) );
	this->Connect( wxID_TOOLNEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnAddObject ) );
	this->Connect( wxID_TOOLEDIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnEditObject ) );
	this->Connect( wxID_TOOLDEL, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnDelObject ) );
	m_lstObjectList->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( MainForm::OnTaskKillFocus ), NULL, this );
	m_lstObjectList->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( MainForm::OnSelectObject ), NULL, this );
	m_chProfile->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MainForm::OnChangeProfile ), NULL, this );
	this->Connect( wxID_TLPROFNEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnAddProfile ) );
	this->Connect( wxID_TLPROFCLONE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnCopyProfile ) );
	this->Connect( wxID_TLPROFDEL, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnDelProfile ) );
	this->Connect( wxID_TLPROFSAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnTaskApply ) );
	this->Connect( wxID_TOOLGO, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnRunTask ) );
	m_bpAddPlugin->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnAddPlugin ), NULL, this );
	m_bpDelPlugin->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnRemovePlugin ), NULL, this );
	m_plgBook->Connect( wxEVT_COMMAND_TREEBOOK_PAGE_CHANGING, wxTreebookEventHandler( MainForm::OnOptionsPageChanging ), NULL, this );
	this->Connect( wxID_TOOLPAUSE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnPauseTask ) );
	this->Connect( wxID_TOOLSTOP, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnCancelTask ) );
	m_lstTaskList->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( MainForm::OnSortItems ), NULL, this );
	m_lstTaskList->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( MainForm::OnTaskSelected ), NULL, this );
	this->Connect( wxID_TLREFRESH, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsRefresh ) );
	this->Connect( wxID_TLFILTER, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsFilter ) );
	m_chTaskFilter->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MainForm::OnReportTskFilter ), NULL, this );
	this->Connect( wxID_TLDATE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsDate ) );
	m_dateFilter->Connect( wxEVT_DATE_CHANGED, wxDateEventHandler( MainForm::OnReportDateFilter ), NULL, this );
	this->Connect( wxID_TLSTATUS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsStatus ) );
	this->Connect( wxID_TLSAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsSave ) );
	m_treeScans->Connect( wxEVT_COMMAND_TREE_ITEM_EXPANDING, wxTreeEventHandler( MainForm::OnReportExpand ), NULL, this );
	m_treeScans->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( MainForm::OnReportSelected ), NULL, this );
	this->Connect( wxID_TLCONNECT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnConnect ) );
	this->Connect( wxID_TLNEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnAddServer ) );
	this->Connect( wxID_TLEDIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnEditServer ) );
	this->Connect( wxID_TLDELETE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnDelServer ) );
	this->Connect( wxID_TLLANGAPPLY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnLangChange ) );
	m_bpStorageApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnStorageChange ), NULL, this );
	this->Connect( wxID_TLPLGRELOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnPlgRefresh ) );
	this->Connect( wxID_TLSRVSTOP, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnSrvStop ) );
	this->Connect( wxID_TLSRVLOG, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnServerLogs ) );
}

MainForm::~MainForm()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MainForm::OnClose ) );
	this->Disconnect( wxID_TOOLNEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnAddObject ) );
	this->Disconnect( wxID_TOOLEDIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnEditObject ) );
	this->Disconnect( wxID_TOOLDEL, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnDelObject ) );
	m_lstObjectList->Disconnect( wxEVT_KILL_FOCUS, wxFocusEventHandler( MainForm::OnTaskKillFocus ), NULL, this );
	m_lstObjectList->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( MainForm::OnSelectObject ), NULL, this );
	m_chProfile->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MainForm::OnChangeProfile ), NULL, this );
	this->Disconnect( wxID_TLPROFNEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnAddProfile ) );
	this->Disconnect( wxID_TLPROFCLONE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnCopyProfile ) );
	this->Disconnect( wxID_TLPROFDEL, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnDelProfile ) );
	this->Disconnect( wxID_TLPROFSAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnTaskApply ) );
	this->Disconnect( wxID_TOOLGO, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnRunTask ) );
	m_bpAddPlugin->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnAddPlugin ), NULL, this );
	m_bpDelPlugin->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnRemovePlugin ), NULL, this );
	m_plgBook->Disconnect( wxEVT_COMMAND_TREEBOOK_PAGE_CHANGING, wxTreebookEventHandler( MainForm::OnOptionsPageChanging ), NULL, this );
	this->Disconnect( wxID_TOOLPAUSE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnPauseTask ) );
	this->Disconnect( wxID_TOOLSTOP, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnCancelTask ) );
	m_lstTaskList->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( MainForm::OnSortItems ), NULL, this );
	m_lstTaskList->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( MainForm::OnTaskSelected ), NULL, this );
	this->Disconnect( wxID_TLREFRESH, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsRefresh ) );
	this->Disconnect( wxID_TLFILTER, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsFilter ) );
	m_chTaskFilter->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MainForm::OnReportTskFilter ), NULL, this );
	this->Disconnect( wxID_TLDATE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsDate ) );
	m_dateFilter->Disconnect( wxEVT_DATE_CHANGED, wxDateEventHandler( MainForm::OnReportDateFilter ), NULL, this );
	this->Disconnect( wxID_TLSTATUS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsStatus ) );
	this->Disconnect( wxID_TLSAVE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnReportsSave ) );
	m_treeScans->Disconnect( wxEVT_COMMAND_TREE_ITEM_EXPANDING, wxTreeEventHandler( MainForm::OnReportExpand ), NULL, this );
	m_treeScans->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( MainForm::OnReportSelected ), NULL, this );
	this->Disconnect( wxID_TLCONNECT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnConnect ) );
	this->Disconnect( wxID_TLNEW, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnAddServer ) );
	this->Disconnect( wxID_TLEDIT, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnEditServer ) );
	this->Disconnect( wxID_TLDELETE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnDelServer ) );
	this->Disconnect( wxID_TLLANGAPPLY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnLangChange ) );
	m_bpStorageApply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MainForm::OnStorageChange ), NULL, this );
	this->Disconnect( wxID_TLPLGRELOAD, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnPlgRefresh ) );
	this->Disconnect( wxID_TLSRVSTOP, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnSrvStop ) );
	this->Disconnect( wxID_TLSRVLOG, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainForm::OnServerLogs ) );
}

ObjDialog::ObjDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSzObjDlg;
	bSzObjDlg = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSzObjData;
	fgSzObjData = new wxFlexGridSizer( 3, 2, 0, 0 );
	fgSzObjData->AddGrowableCol( 1 );
	fgSzObjData->AddGrowableRow( 2 );
	fgSzObjData->SetFlexibleDirection( wxBOTH );
	fgSzObjData->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_stObjName;
	m_stObjName = new wxStaticText( this, wxID_ANY, _("Object name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stObjName->Wrap( -1 );
	fgSzObjData->Add( m_stObjName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_txtObjName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSzObjData->Add( m_txtObjName, 0, wxALL|wxEXPAND, 5 );

	wxStaticText* m_stBaseURL;
	m_stBaseURL = new wxStaticText( this, wxID_ANY, _("Base URL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stBaseURL->Wrap( -1 );
	fgSzObjData->Add( m_stBaseURL, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_txtBaseURL = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSzObjData->Add( m_txtBaseURL, 0, wxALL|wxEXPAND, 5 );

	bSzObjDlg->Add( fgSzObjData, 1, wxEXPAND, 5 );

	m_sdbSizer2 = new wxStdDialogButtonSizer();
	m_sdbSizer2OK = new wxButton( this, wxID_OK );
	m_sdbSizer2->AddButton( m_sdbSizer2OK );
	m_sdbSizer2Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer2->AddButton( m_sdbSizer2Cancel );
	m_sdbSizer2->Realize();
	bSzObjDlg->Add( m_sdbSizer2, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( bSzObjDlg );
	this->Layout();

	// Connect Events
	m_sdbSizer2OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ObjDialog::OnOK ), NULL, this );
}

ObjDialog::~ObjDialog()
{
	// Disconnect Events
	m_sdbSizer2OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ObjDialog::OnOK ), NULL, this );
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

PlgSelector::PlgSelector( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );

	m_plgTree = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_SINGLE );
	bSizer24->Add( m_plgTree, 1, wxALL|wxEXPAND, 2 );

	m_sdbSizer3 = new wxStdDialogButtonSizer();
	m_sdbSizer3OK = new wxButton( this, wxID_OK );
	m_sdbSizer3->AddButton( m_sdbSizer3OK );
	m_sdbSizer3Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer3->AddButton( m_sdbSizer3Cancel );
	m_sdbSizer3->Realize();
	bSizer24->Add( m_sdbSizer3, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( bSizer24 );
	this->Layout();

	// Connect Events
	m_sdbSizer3OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PlgSelector::OnOK ), NULL, this );
}

PlgSelector::~PlgSelector()
{
	// Disconnect Events
	m_sdbSizer3OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PlgSelector::OnOK ), NULL, this );
}

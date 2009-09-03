///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wxGuiData.h"

#include "../../images/btnCopy.xpm"
#include "../../images/btnEdit.xpm"
#include "../../images/flsave.xpm"
#include "../../images/opts_audit.xpm"
#include "srlogo.xpm"

///////////////////////////////////////////////////////////////////////////

SetupDlg::SetupDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 700,400 ), wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );
	
	wxBoxSizer* bSzFrame;
	bSzFrame = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSzPanel;
	bSzPanel = new wxBoxSizer( wxHORIZONTAL );
	
	wxGridBagSizer* gbSizer1;
	gbSizer1 = new wxGridBagSizer( 0, 0 );
	gbSizer1->AddGrowableCol( 2 );
	gbSizer1->AddGrowableRow( 2 );
	gbSizer1->SetFlexibleDirection( wxBOTH );
	gbSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	gbSizer1->SetMinSize( wxSize( 300,-1 ) ); 
	m_stVerLabel = new wxStaticText( this, wxID_ANY, _("SimpleReporter version:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stVerLabel->Wrap( -1 );
	gbSizer1->Add( m_stVerLabel, wxGBPosition( 0, 0 ), wxGBSpan( 1, 2 ), wxALL, 5 );
	
	m_stVersion = new wxStaticText( this, wxID_ANY, _("0.1.1"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stVersion->Wrap( -1 );
	gbSizer1->Add( m_stVersion, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALL, 5 );
	
	wxStaticText* m_stFile;
	m_stFile = new wxStaticText( this, wxID_ANY, _("Use template"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stFile->Wrap( -1 );
	gbSizer1->Add( m_stFile, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_fpTemplate = new wxFilePickerCtrl( this, wxID_ANY, wxEmptyString, _("Select a file"), wxT("*.xml"), wxDefaultPosition, wxSize( -1,-1 ), wxFLP_DEFAULT_STYLE );
	gbSizer1->Add( m_fpTemplate, wxGBPosition( 1, 1 ), wxGBSpan( 1, 2 ), wxALL|wxEXPAND, 5 );
	
	m_bmpLogo = new wxStaticBitmap( this, wxID_ANY, wxBitmap( srlogo_xpm ), wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer1->Add( m_bmpLogo, wxGBPosition( 2, 0 ), wxGBSpan( 1, 3 ), wxALIGN_CENTER|wxALL, 5 );
	
	wxStaticText* m_stCopyright;
	m_stCopyright = new wxStaticText( this, wxID_ANY, _("Copyright (c) 2009. Andrew \"Stinger\" Abramov"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stCopyright->Wrap( -1 );
	gbSizer1->Add( m_stCopyright, wxGBPosition( 3, 0 ), wxGBSpan( 1, 3 ), wxALIGN_CENTER|wxALL, 5 );
	
	m_hyperlink = new wxHyperlinkCtrl( this, wxID_ANY, _("Lab18 website"), wxT("http://www.lab18.net"), wxDefaultPosition, wxDefaultSize, wxHL_ALIGN_CENTRE|wxHL_DEFAULT_STYLE );
	
	m_hyperlink->SetHoverColour( wxColour( 0, 128, 192 ) );
	m_hyperlink->SetNormalColour( wxColour( 0, 128, 192 ) );
	m_hyperlink->SetVisitedColour( wxColour( 0, 128, 192 ) );
	gbSizer1->Add( m_hyperlink, wxGBPosition( 4, 0 ), wxGBSpan( 1, 3 ), wxALIGN_CENTER|wxALL, 5 );
	
	bSzPanel->Add( gbSizer1, 0, 0, 0 );
	
	m_panel1 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );
	
	m_toolBar1 = new wxToolBar( m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL ); 
	m_toolBar1->AddTool( wxID_ANY, wxEmptyString, wxBitmap( btnCopy_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Copy"), wxEmptyString );
	m_toolBar1->AddTool( wxID_ANY, wxEmptyString, wxBitmap( btnEdit_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Cut"), wxEmptyString );
	m_toolBar1->AddTool( wxID_ANY, wxEmptyString, wxBitmap( opts_audit_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Paste"), wxEmptyString );
	m_toolBar1->AddSeparator();
	m_toolBar1->AddTool( wxID_ANY, wxEmptyString, wxBitmap( flsave_xpm ), wxNullBitmap, wxITEM_NORMAL, _("Save current file"), wxEmptyString );
	m_toolBar1->Realize();
	
	bSizer3->Add( m_toolBar1, 0, wxEXPAND, 5 );
	
	m_scEditor = new wxScintilla( m_panel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString );
	m_scEditor->SetUseTabs( false );
	m_scEditor->SetTabWidth( 4 );
	m_scEditor->SetIndent( 4 );
	m_scEditor->SetTabIndents( true );
	m_scEditor->SetBackSpaceUnIndents( true );
	m_scEditor->SetViewEOL( false );
	m_scEditor->SetViewWhiteSpace( false );
	m_scEditor->SetMarginWidth( 2, 0 );
	m_scEditor->SetIndentationGuides( true );
	m_scEditor->SetMarginType( 1, wxSCI_MARGIN_SYMBOL );
	m_scEditor->SetMarginMask( 1, wxSCI_MASK_FOLDERS );
	m_scEditor->SetMarginWidth( 1, 16);
	m_scEditor->SetMarginSensitive( 1, true );
	m_scEditor->SetProperty( wxT("fold"), wxT("1") );
	m_scEditor->SetFoldFlags( wxSCI_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSCI_FOLDFLAG_LINEAFTER_CONTRACTED );
	m_scEditor->SetMarginType( 0, wxSCI_MARGIN_NUMBER );
	m_scEditor->SetMarginWidth( 0, m_scEditor->TextWidth( wxSCI_STYLE_LINENUMBER, wxT("_99999") ) );
	m_scEditor->MarkerDefine( wxSCI_MARKNUM_FOLDER, wxSCI_MARK_BOXPLUS );
	m_scEditor->MarkerSetBackground( wxSCI_MARKNUM_FOLDER, wxColour( wxT("BLACK") ) );
	m_scEditor->MarkerSetForeground( wxSCI_MARKNUM_FOLDER, wxColour( wxT("WHITE") ) );
	m_scEditor->MarkerDefine( wxSCI_MARKNUM_FOLDEROPEN, wxSCI_MARK_BOXMINUS );
	m_scEditor->MarkerSetBackground( wxSCI_MARKNUM_FOLDEROPEN, wxColour( wxT("BLACK") ) );
	m_scEditor->MarkerSetForeground( wxSCI_MARKNUM_FOLDEROPEN, wxColour( wxT("WHITE") ) );
	m_scEditor->MarkerDefine( wxSCI_MARKNUM_FOLDERSUB, wxSCI_MARK_EMPTY );
	m_scEditor->MarkerDefine( wxSCI_MARKNUM_FOLDEREND, wxSCI_MARK_BOXPLUS );
	m_scEditor->MarkerSetBackground( wxSCI_MARKNUM_FOLDEREND, wxColour( wxT("BLACK") ) );
	m_scEditor->MarkerSetForeground( wxSCI_MARKNUM_FOLDEREND, wxColour( wxT("WHITE") ) );
	m_scEditor->MarkerDefine( wxSCI_MARKNUM_FOLDEROPENMID, wxSCI_MARK_BOXMINUS );
	m_scEditor->MarkerSetBackground( wxSCI_MARKNUM_FOLDEROPENMID, wxColour( wxT("BLACK") ) );
	m_scEditor->MarkerSetForeground( wxSCI_MARKNUM_FOLDEROPENMID, wxColour( wxT("WHITE") ) );
	m_scEditor->MarkerDefine( wxSCI_MARKNUM_FOLDERMIDTAIL, wxSCI_MARK_EMPTY );
	m_scEditor->MarkerDefine( wxSCI_MARKNUM_FOLDERTAIL, wxSCI_MARK_EMPTY );
	m_scEditor->SetSelBackground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_scEditor->SetSelForeground( true, wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT ) );
	bSizer3->Add( m_scEditor, 1, wxEXPAND | wxALL, 0 );
	
	m_panel1->SetSizer( bSizer3 );
	m_panel1->Layout();
	bSizer3->Fit( m_panel1 );
	bSzPanel->Add( m_panel1, 1, wxEXPAND | wxALL, 0 );
	
	bSzFrame->Add( bSzPanel, 1, wxEXPAND, 5 );
	
	
	bSzFrame->Add( 0, 5, 0, wxEXPAND, 5 );
	
	m_sdbButtons = new wxStdDialogButtonSizer();
	m_sdbButtonsOK = new wxButton( this, wxID_OK );
	m_sdbButtons->AddButton( m_sdbButtonsOK );
	m_sdbButtonsCancel = new wxButton( this, wxID_CANCEL );
	m_sdbButtons->AddButton( m_sdbButtonsCancel );
	m_sdbButtons->Realize();
	bSzFrame->Add( m_sdbButtons, 0, wxEXPAND, 5 );
	
	this->SetSizer( bSzFrame );
	this->Layout();
	
	// Connect Events
	m_fpTemplate->Connect( wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler( SetupDlg::OnFileOpen ), NULL, this );
	this->Connect( wxID_ANY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( SetupDlg::OnTxtCopy ) );
	this->Connect( wxID_ANY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( SetupDlg::OnTxtCut ) );
	this->Connect( wxID_ANY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( SetupDlg::OnTxtPaste ) );
	this->Connect( wxID_ANY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( SetupDlg::OnTxtSave ) );
	m_sdbButtonsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SetupDlg::OnCancel ), NULL, this );
	m_sdbButtonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SetupDlg::OnOK ), NULL, this );
}

SetupDlg::~SetupDlg()
{
	// Disconnect Events
	m_fpTemplate->Disconnect( wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler( SetupDlg::OnFileOpen ), NULL, this );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( SetupDlg::OnTxtCopy ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( SetupDlg::OnTxtCut ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( SetupDlg::OnTxtPaste ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( SetupDlg::OnTxtSave ) );
	m_sdbButtonsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SetupDlg::OnCancel ), NULL, this );
	m_sdbButtonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SetupDlg::OnOK ), NULL, this );
}

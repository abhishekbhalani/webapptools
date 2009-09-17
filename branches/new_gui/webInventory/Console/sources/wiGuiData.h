///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __wiGuiData__
#define __wiGuiData__

#include <wx/intl.h>

#include <wx/gdicmn.h>
#include <wx/aui/auibook.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/toolbar.h>
#include <wx/listctrl.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/panel.h>
#include <wx/listbook.h>
#include <wx/imaglist.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/propgrid/manager.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/treectrl.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

#define wxID_TOOLNEW 1000
#define wxID_TOOLEDIT 1001
#define wxID_TOOLDEL 1002
#define wxID_TLPROFNEW 1003
#define wxID_TLPROFCLONE 1004
#define wxID_TLPROFDEL 1005
#define wxID_TLPROFSAVE 1006
#define wxID_TOOLGO 1007
#define wxID_TOOLPAUSE 1008
#define wxID_TOOLSTOP 1009
#define wxID_TLREFRTSKLOG 1010
#define wxID_TLSAVETSKLOG 1011
#define wxID_TLCONNECT 1012
#define wxID_TLNEW 1013
#define wxID_TLEDIT 1014
#define wxID_TLDELETE 1015
#define wxID_TLLANGAPPLY 1016
#define wxID_TLPLGRELOAD 1017
#define wxID_TLSRVRESTART 1018
#define wxID_TLSRVSTOP 1019
#define wxID_TLSRVLOG 1020
#define wxID_PLUG_INFO 1021
#define wxID_TLREFRESH 1022
#define wxID_TLFILTER 1023
#define wxID_TLDATE 1024
#define wxID_TLSTATUS 1025
#define wxID_TLSAVE 1026

///////////////////////////////////////////////////////////////////////////////
/// Class MainForm
///////////////////////////////////////////////////////////////////////////////
class MainForm : public wxFrame
{
	private:

	protected:
		wxAuiNotebook* m_mainnb;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ){ event.Skip(); }


	public:
		MainForm( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("WebAudit Framework"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1070,660 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~MainForm();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Tasks
///////////////////////////////////////////////////////////////////////////////
class Tasks : public wxPanel
{
	private:

	protected:
		wxSplitterWindow* m_splitTasks;
		wxPanel* m_panObjects;
		wxToolBar* m_toolBarObject;
		wxListCtrl* m_lstObjectList;
		wxBoxSizer* bSzProfile;
		wxToolBar* m_toolBarProf;
		wxChoice* m_chProfile;
		wxBitmapButton* m_bpAddPlugin;
		wxBitmapButton* m_bpDelPlugin;
		wxTextCtrl* m_txtProfName;
		wxListbook* m_plgBook;
		wxPanel* m_panTasks;
		wxToolBar* m_toolBarTasks;
		wxSpinCtrl* m_spinLinesTsk;
		wxCheckBox* m_chAppendTsk;
		wxSplitterWindow* m_splitLogs;
		wxListCtrl* m_lstTaskList;
		wxRichTextCtrl* m_rtTask;

		// Virtual event handlers, overide them in your derived class
		virtual void OnAddObject( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEditObject( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDelObject( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnTaskKillFocus( wxFocusEvent& event ){ event.Skip(); }
		virtual void OnSelectObject( wxListEvent& event ){ event.Skip(); }
		virtual void OnChangeProfile( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAddProfile( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCopyProfile( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDelProfile( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnTaskApply( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRunTask( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAddPlugin( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRemovePlugin( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnOptionsPageChanging( wxListbookEvent& event ){ event.Skip(); }
		virtual void OnPauseTask( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCancelTask( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSortItems( wxListEvent& event ){ event.Skip(); }
		virtual void OnTaskSelected( wxListEvent& event ){ event.Skip(); }


	public:
		Tasks( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 753,512 ), long style = wxTAB_TRAVERSAL );
		~Tasks();
		void m_splitTasksOnIdle( wxIdleEvent& )
		{
		m_splitTasks->SetSashPosition( 0 );
		m_splitTasks->Disconnect( wxEVT_IDLE, wxIdleEventHandler( Tasks::m_splitTasksOnIdle ), NULL, this );
		}

		void m_splitLogsOnIdle( wxIdleEvent& )
		{
		m_splitLogs->SetSashPosition( 300 );
		m_splitLogs->Disconnect( wxEVT_IDLE, wxIdleEventHandler( Tasks::m_splitLogsOnIdle ), NULL, this );
		}


};

///////////////////////////////////////////////////////////////////////////////
/// Class Settings
///////////////////////////////////////////////////////////////////////////////
class Settings : public wxPanel
{
	private:

	protected:
		wxStaticText* m_stConVers;
		wxStaticText* m_stConVersData;
		wxStaticText* m_stServer;
		wxChoice* m_chServers;
		wxToolBar* m_toolBarSrv;
		wxStaticText* m_stSrvVers;
		wxStaticText* m_stSrvVersData;
		wxStaticText* m_stLang;
		wxChoice* m_chLangs;
		wxToolBar* m_toolBarLang;
		wxStaticText* m_stLangRestart;
		wxCheckBox* m_chLogging;

		wxStaticLine* m_staticline2;
		wxPanel* m_pnServer;
		wxFlexGridSizer* fgSrvSizer;
		wxStaticText* m_staticText18;
		wxChoice* m_chStorage;
		wxBitmapButton* m_bpStorageApply;

		wxToolBar* m_tbServer;
		wxStaticText* m_staticText19;
		wxPropertyGridManager* m_pgrdManager;


		// Virtual event handlers, overide them in your derived class
		virtual void OnChangeSrv( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnConnect( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnAddServer( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnEditServer( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDelServer( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnLangChange( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnStorageChange( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnPlgRefresh( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSrvRestart( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnSrvStop( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnServerLogs( wxCommandEvent& event ){ event.Skip(); }


	public:
		Settings( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 706,549 ), long style = wxTAB_TRAVERSAL );
		~Settings();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Reports
///////////////////////////////////////////////////////////////////////////////
class Reports : public wxPanel
{
	private:

	protected:
		wxSplitterWindow* m_split;
		wxPanel* m_panRepTree;
		wxToolBar* m_toolBarFilter;
		wxChoice* m_chTaskFilter;
		wxDatePickerCtrl* m_dateFilter;
		wxToolBar* m_tbRepPlugs;
		wxChoice* m_chReporters;
		wxTreeCtrl* m_treeScans;
		wxPanel* m_panRepData;
		wxRichTextCtrl* m_richText2;

		// Virtual event handlers, overide them in your derived class
		virtual void OnReportsRefresh( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnReportsFilter( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnReportTskFilter( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnReportsDate( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnReportDateFilter( wxDateEvent& event ){ event.Skip(); }
		virtual void OnReportsStatus( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnReportsSave( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnReporterChanged( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnReporterApply( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnReporterLoad( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnReportExpand( wxTreeEvent& event ){ event.Skip(); }
		virtual void OnReportSelected( wxTreeEvent& event ){ event.Skip(); }


	public:
		Reports( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 962,586 ), long style = wxTAB_TRAVERSAL );
		~Reports();
		void m_splitOnIdle( wxIdleEvent& )
		{
		m_split->SetSashPosition( 400 );
		m_split->Disconnect( wxEVT_IDLE, wxIdleEventHandler( Reports::m_splitOnIdle ), NULL, this );
		}


};

///////////////////////////////////////////////////////////////////////////////
/// Class PluginSettings
///////////////////////////////////////////////////////////////////////////////
class PluginSettings : public wxPanel
{
	private:

	protected:
		wxPropertyGrid* m_props;

	public:
		PluginSettings( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 363,300 ), long style = wxTAB_TRAVERSAL );
		~PluginSettings();

};

///////////////////////////////////////////////////////////////////////////////
/// Class ObjDialog
///////////////////////////////////////////////////////////////////////////////
class ObjDialog : public wxDialog
{
	private:

	protected:
		wxStdDialogButtonSizer* m_sdbSizer2;
		wxButton* m_sdbSizer2OK;
		wxButton* m_sdbSizer2Cancel;

		// Virtual event handlers, overide them in your derived class
		virtual void OnOK( wxCommandEvent& event ){ event.Skip(); }


	public:
		wxTextCtrl* m_txtObjName;
		wxTextCtrl* m_txtBaseURL;
		ObjDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Object"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 279,130 ), long style = wxDEFAULT_DIALOG_STYLE );
		~ObjDialog();

};

///////////////////////////////////////////////////////////////////////////////
/// Class ServDialog
///////////////////////////////////////////////////////////////////////////////
class ServDialog : public wxDialog
{
	private:

	protected:
		wxStaticText* m_stName;
		wxStaticText* m_stServName;
		wxStaticText* m_stPort;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;

		// Virtual event handlers, overide them in your derived class
		virtual void OnOK( wxCommandEvent& event ){ event.Skip(); }


	public:
		wxTextCtrl* m_txtName;
		wxTextCtrl* m_txtHostname;
		wxTextCtrl* m_txtSrvPort;
		ServDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Connection"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 296,156 ), long style = wxDEFAULT_DIALOG_STYLE );
		~ServDialog();

};

///////////////////////////////////////////////////////////////////////////////
/// Class PlgSelector
///////////////////////////////////////////////////////////////////////////////
class PlgSelector : public wxDialog
{
	private:

	protected:
		wxStdDialogButtonSizer* m_sdbSizer3;
		wxButton* m_sdbSizer3OK;
		wxButton* m_sdbSizer3Cancel;

		// Virtual event handlers, overide them in your derived class
		virtual void OnOK( wxCommandEvent& event ){ event.Skip(); }


	public:
		wxTreeCtrl* m_plgTree;
		PlgSelector( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Select plugin"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 331,350 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~PlgSelector();

};

#endif //__wiGuiData__

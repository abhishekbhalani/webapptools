///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __wxGuiData__
#define __wxGuiData__

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/filepicker.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/hyperlink.h>
#include <wx/gbsizer.h>
#include <wx/toolbar.h>
#include <wx/wxscintilla/wxscintilla.h>
#ifdef __VISUALC__
#include <wx/link_additions.h>
#endif //__VISUALC__
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class SetupDlg
///////////////////////////////////////////////////////////////////////////////
class SetupDlg : public wxFrame 
{
	private:
	
	protected:
		wxStaticText* m_stVerLabel;
		wxStaticText* m_stVersion;
		wxFilePickerCtrl* m_fpTemplate;
		wxStaticBitmap* m_bmpLogo;
		wxHyperlinkCtrl* m_hyperlink;
		wxPanel* m_panel1;
		wxToolBar* m_toolBar1;
		wxScintilla* m_scEditor;
		
		wxStdDialogButtonSizer* m_sdbButtons;
		wxButton* m_sdbButtonsOK;
		wxButton* m_sdbButtonsCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnFileOpen( wxFileDirPickerEvent& event ){ event.Skip(); }
		virtual void OnTxtCopy( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnTxtCut( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnTxtPaste( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnTxtSave( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnOK( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		SetupDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("SimpleReporter settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,420 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~SetupDlg();
	
};

#endif //__wxGuiData__

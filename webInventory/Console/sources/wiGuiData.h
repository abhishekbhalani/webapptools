///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __wiGuiData__
#define __wiGuiData__

class wiStatBar;

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/listctrl.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/panel.h>
#include <wx/aui/auibook.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class MainForm
///////////////////////////////////////////////////////////////////////////////
class MainForm : public wxFrame 
{
	private:
	
	protected:
		wxAuiNotebook* m_mainnb;
		wxPanel* m_pTasks;
		wxStaticText* m_staticText1;
		
		wxBitmapButton* m_bpButton51;
		wxBitmapButton* m_bpButton511;
		wxBitmapButton* m_bpButton2;
		wxStaticText* m_staticText2;
		
		wxStaticText* m_staticText4;
		wxChoice* m_choice1;
		wxListCtrl* m_listCtrl2;
		wxRichTextCtrl* m_richText1;
		wxStaticText* m_staticText3;
		
		wxBitmapButton* m_bpButton5;
		wxBitmapButton* m_bpButton11;
		wxBitmapButton* m_bpButton21;
		wxListCtrl* m_listCtrl21;
		wxPanel* m_pReports;
		wxPanel* m_pSettings;
		wiStatBar* m_statusBar1;
	
	public:
		MainForm( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("WebInventory"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 1070,660 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~MainForm();
	
};

#endif //__wiGuiData__

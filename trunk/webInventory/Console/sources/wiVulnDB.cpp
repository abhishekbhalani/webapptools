/*
    webInventory is the web-application audit programm
    Copyright (C) 2009 Andrew Abramov stinger911@gmail.com

    This file is part of webInventory

    webInventory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webInventory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webInventory.  If not, see <http://www.gnu.org/licenses/>.
*//************************************************************
 * @file      wiVulnDB.cpp
 * @brief     Code for MainForm class
 * @author    Tursas Pohielainen (tursasp@gmail.com)
 * @date      05.10.2009
 **************************************************************/
#include "wiVulnDB.h"
#include "sciLexer.h"

wiVulnDB::wiVulnDB( wxWindow* parent )
:
VulnDB( parent )
{
    m_bDataChanged = false;
   	m_scDesc->SetProperty( wxT("fold"), wxT("1") );
   	m_scDesc->SetProperty (wxT("fold.html"), wxT("1") );
   	m_scDesc->SetProperty (wxT("fold.comment"), wxT("1") );
    m_scDesc->SetProperty (wxT("fold.html.preprocessor"), wxT("1") );
    m_scDesc->SetFoldFlags(wxSCI_FOLDFLAG_LINEBEFORE_CONTRACTED |
                  wxSCI_FOLDFLAG_LINEAFTER_CONTRACTED);

    m_scDesc->SetTabWidth (4);
    m_scDesc->SetUseTabs (false);
    m_scDesc->SetTabIndents (true);
    m_scDesc->SetBackSpaceUnIndents (true);
    m_scDesc->SetIndent ( 4 );

    m_scDesc->SetLexer(wxSCI_LEX_XML);
	m_scDesc->SetLexerLanguage(wxT("xml")); // hypertext
	wxFont src_font(9, wxTELETYPE, wxNORMAL, wxNORMAL, false,wxT("Courier"));
	m_scDesc->SetFont(src_font);
	m_scDesc->SetKeyWords(0, keywordsHTML);
	m_scDesc->SetKeyWords(1, keywordsJavaScript);
	m_scDesc->SetProperty(wxT("asp.default.language"), wxT("1"));
	m_scDesc->SetWrapMode(wxSCI_WRAP_WORD);
	for (int i = 0; ; i++) {
		if (htmlLexerColors[i].styleCode == LEXER_LAST_RECORD) {
			break;
		}
		m_scDesc->StyleSetForeground(htmlLexerColors[i].styleCode, wxColor(wxString(htmlLexerColors[i].styleColor)));
		m_scDesc->StyleSetFont(htmlLexerColors[i].styleCode, src_font);
	}
	m_scDesc->StyleSetBold(wxSCI_H_TAG, true);
	m_scDesc->StyleSetBold(wxSCI_H_TAGUNKNOWN, true);
	m_scDesc->StyleSetBold(wxSCI_HJ_KEYWORD, true);
	m_scDesc->StyleSetBold(wxSCI_HJA_KEYWORD, true);

}

void wiVulnDB::OnTreeRefresh( wxCommandEvent& event )
{
    if ( !ContinueUnsaved() ) {
        return;
    }
	// TODO: Implement OnTreeRefresh
}

void wiVulnDB::OnDbSlice( wxCommandEvent& event )
{
    if ( !ContinueUnsaved() ) {
        return;
    }
	// TODO: Implement OnDbSlice
}

void wiVulnDB::OnAddObj( wxCommandEvent& event )
{
    if ( !ContinueUnsaved() ) {
        return;
    }
	// TODO: Implement OnAddObj
}

void wiVulnDB::OnCloneObj( wxCommandEvent& event )
{
    if ( !ContinueUnsaved() ) {
        return;
    }
	// TODO: Implement OnCloneObj
}

void wiVulnDB::OnDelObj( wxCommandEvent& event )
{
    if ( !ContinueUnsaved() ) {
        return;
    }
	// TODO: Implement OnDelObj
}

void wiVulnDB::OnSaveObj( wxCommandEvent& event )
{
	// TODO: Implement OnSaveObj
}

void wiVulnDB::OnVulnerSelect( wxTreeEvent& event )
{
    if ( !ContinueUnsaved() ) {
        event.Veto();
        return;
    }
	// TODO: Implement OnVulnerSelect
}

void wiVulnDB::OnDataChanged( wxCommandEvent& event )
{
	m_bDataChanged = true;
	m_toolBar->EnableTool(wxID_SAVE, true);
}

bool wiVulnDB::ContinueUnsaved()
{
    // check if m_scDesc is modified
    if (m_bDataChanged) {
        int resp = wxMessageBox(_("Description was changed. Do you want to save it?"), _("WebAudit Framework"), wxICON_QUESTION | wxYES_NO | wxCANCEL);
        if (resp == wxCANCEL) {
            return false;
        }
        if (resp == wxYES) {
            // save current vulner
        }
    }
    return true;
}

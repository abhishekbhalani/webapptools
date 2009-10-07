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
#include <wx/wupdlock.h>
#include <wx/mstream.h>
#include <wx/xml/xml.h>
#include "wiVulnDB.h"
#include "sciLexer.h"

// tasklist images
#include "../images/tree_unk.xpm"       // gray image
#include "../images/tree_blue.xpm"      // blue image
#include "../images/tree_yes.xpm"       // green image
#include "../images/tree_lgr.xpm"       // light green
#include "../images/tree_yell.xpm"      // yellow image
#include "../images/tree_orange.xpm"    // orange image
#include "../images/tree_no.xpm"        // red image
#include "../images/opts_vulner.xpm"

WX_DECLARE_STRING_HASH_MAP( wxTreeItemId, wiVulnerIdHash );

// !!!DEBUG
wxString sampleXML = wxT("<vdescs>\
    <vdesc id='CVE-2008-5515' severity='1'>Directory traversal in Apache</vdesc>\
    <vdesc id='CVE-2008-5516' severity='2'>Directory traversal in Tomcat</vdesc>\
    <vdesc id='CVE-2008-5517' severity='3'>Directory traversal in RequestDispatcher</vdesc>\
    <vdesc id='NMAP-0001' severity='0'>Nmap services discovery</vdesc>\
    <vdesc id='NMAP-0002' severity='5'>Nmap OS discovery</vdesc>\
    <vdesc id='NMAP-0003' severity='4'>Nmap ports discovery</vdesc>\
</vdescs>");

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
	wxImageList* imageList = new wxImageList;
	imageList->Add(wxIcon(tree_unk_xpm));
	imageList->Add(wxIcon(tree_blue_xpm));
	imageList->Add(wxIcon(tree_yes_xpm));
	imageList->Add(wxIcon(tree_lgr_xpm));
	imageList->Add(wxIcon(tree_yell_xpm));
	imageList->Add(wxIcon(tree_orange_xpm));
	imageList->Add(wxIcon(tree_no_xpm));
	imageList->Add(wxIcon(opts_vulner_xpm));
	m_treeVDB->AssignImageList(imageList);

	DrawTree();
}

void wiVulnDB::OnTreeRefresh( wxCommandEvent& event )
{
    if ( !ContinueUnsaved() ) {
        return;
    }
	DrawTree();
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

void wiVulnDB::DrawTree()
{
    wxWindowUpdateLocker noUpdates(m_treeVDB);
    wiVulnerIdHash hashMap;
    wiVulnerIdHash::iterator  it;
    wxTreeItemId parent, rootID;
    wxString strId, strTitle, strLev;
    long severity;

    //wxString dt = FRAME_WINDOW->DoClientCommand(wxT("getvdesclist"), wxT(""));
    wxString dt = sampleXML;
    if (dt != wxT("")) {
        wxCharBuffer buff = dt.utf8_str();
        int buffLen = strlen(buff.data());
        wxMemoryInputStream xmlStream(buff.data(), buffLen);
        //wxLogNull logger;
        wxXmlDocument doc;

        if ( doc.Load(xmlStream) ) {
            m_treeVDB->DeleteAllItems();
            rootID = m_treeVDB->AddRoot(wxT("Vulnerabilities"));
            wxXmlNode *root, *chld;
            root = doc.GetRoot();
            if (root != NULL) {
                chld = root->GetChildren();
                while (chld != NULL) {
                    strId = chld->GetPropVal(wxT("id"), wxT(""));
                    strLev = chld->GetPropVal(wxT("severity"), wxT(""));
                    if (strLev.ToLong(&severity)) {
                        severity++;
                        if (severity < 0 || severity > 6) {
                            severity = 0;
                        }
                    }
                    else {
                        severity = 0;
                    }
                    strTitle = chld->GetNodeContent();
                    if (strId != wxT("")) {
                        wxString plgName;
                        plgName = strId.BeforeFirst(wxT('-'));
                        if (plgName.IsEmpty()) {
                            plgName = strId;
                        }
                        it = hashMap.find(plgName);
                        if (it == hashMap.end() ) {
                            parent = m_treeVDB->AppendItem(rootID, plgName, 7);
                            hashMap[plgName] = parent;
                        }
                        else {
                            parent = it->second;
                        }
                        strId = wxString::Format(wxT("%s: %s"), strId.c_str(), strTitle.c_str());
                        m_treeVDB->AppendItem(parent, strId, severity);
                    }
                    chld = chld->GetNext();
                }
            }
        }
    }
}

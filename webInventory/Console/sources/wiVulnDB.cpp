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
#include "Crypto.h"

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

class wiVulnTreeData : public wxTreeItemData
{
    public:
        wxString    vulnID;
        wxString    pluginID;
};

// !!!DEBUG
wxString sampleXML = wxT("<vdescs>\
    <vdesc id='CVE-2008-5515' severity='1'>Directory traversal in Apache</vdesc>\
    <vdesc id='CVE-2008-5516' severity='2'>Directory traversal in Tomcat</vdesc>\
    <vdesc id='CVE-2008-5517' severity='3'>Directory traversal in RequestDispatcher</vdesc>\
    <vdesc id='NMAP-0001' severity='0'>Nmap services discovery</vdesc>\
    <vdesc id='NMAP-0002' severity='5'>Nmap OS discovery</vdesc>\
    <vdesc id='NMAP-0003' severity='4'>Nmap ports discovery</vdesc>\
</vdescs>");
wxString vdescXML = wxT("<vdesc id='CVE-2008-5515'>\
<title>Directory traversal in Apache</title>\
<severity>1</severity>\
<short_desc>Short description\nin two lines</short_desc>\
<description>&lt;root&gt;\n&lt;xml_stylesheet&gt;\n&lt;body&gt;\nText\n&lt;/body&gt;\n&lt;/xml_stylesheet&gt;\n&lt;/root&gt;\n</description>\
</vdesc>");

wiVulnDB::wiVulnDB( wxWindow* parent )
:
VulnDB( parent )
{
    m_bDataChanged = false;
    m_forceSelect = false;
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
	wxImageList* imageList = new wxImageList(16, 16);
	imageList->Add(wxBitmap(tree_unk_xpm));
	imageList->Add(wxBitmap(tree_blue_xpm));
	imageList->Add(wxBitmap(tree_yes_xpm));
	imageList->Add(wxBitmap(tree_lgr_xpm));
	imageList->Add(wxBitmap(tree_yell_xpm));
	imageList->Add(wxBitmap(tree_orange_xpm));
	imageList->Add(wxBitmap(tree_no_xpm));
	imageList->Add(wxBitmap(opts_vulner_xpm));
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
    wxTreeItemId idTree, itemID;
    wiVulnTreeData* itemData;

    if ( !ContinueUnsaved() ) {
        return;
    }
    idTree = m_treeVDB->GetSelection();
    if (!idTree.IsOk()) {
        return;
    }
    itemData = (wiVulnTreeData*)m_treeVDB->GetItemData(idTree);
    if (!itemData) {
        return;
    }

    m_txtLabel->SetValue(itemData->pluginID);
    m_chSeverity->Select(0);
    m_txtTiltle->Clear();
    m_txtShort->Clear();
    m_scDesc->ClearAll();
    m_bDataChanged = false;

    if (itemData->vulnID != wxEmptyString) {
        idTree = m_treeVDB->GetItemParent(idTree);
    }
    itemID = m_treeVDB->AppendItem(idTree, itemData->pluginID, 1);
    itemData = new wiVulnTreeData;
    itemData->vulnID = m_txtLabel->GetValue();
    itemData->pluginID = itemData->vulnID;
    m_treeVDB->SetItemData(itemID, itemData);
    m_forceSelect = true;
    m_treeVDB->SelectItem(itemID);
    m_forceSelect = false;
}

void wiVulnDB::OnCloneObj( wxCommandEvent& event )
{
    wxTreeItemId idTree, itemID;
    wiVulnTreeData* itemData;

    if ( !ContinueUnsaved() ) {
        return;
    }
    idTree = m_treeVDB->GetSelection();
    if (!idTree.IsOk()) {
        return;
    }
    itemData = (wiVulnTreeData*)m_treeVDB->GetItemData(idTree);
    if (!itemData || itemData->vulnID.IsEmpty() ) {
        return;
    }

    m_txtLabel->SetValue(itemData->pluginID);

    idTree = m_treeVDB->GetItemParent(idTree);
    itemID = m_treeVDB->AppendItem(idTree, itemData->pluginID, m_chSeverity->GetSelection() + 1);
    itemData = new wiVulnTreeData;
    itemData->vulnID = m_txtLabel->GetValue();
    itemData->pluginID = itemData->vulnID;
    m_treeVDB->SetItemData(itemID, itemData);
    m_forceSelect = true;
    m_treeVDB->SelectItem(itemID);
    m_forceSelect = false;
}

void wiVulnDB::OnDelObj( wxCommandEvent& event )
{
    wxTreeItemId idTree, itemID;
    wiVulnTreeData* itemData;

    if ( !ContinueUnsaved() ) {
        return;
    }
    idTree = m_treeVDB->GetSelection();
    if (!idTree.IsOk()) {
        return;
    }
    itemData = (wiVulnTreeData*)m_treeVDB->GetItemData(idTree);
    if (!itemData || itemData->vulnID.IsEmpty() ) {
        return;
    }
    wxString msg = wxString::Format(_("Are you sure to delete vulnerability %s?"), itemData->vulnID.c_str());
    int res = wxMessageBox(msg, _("WebAudit Framework"), wxICON_WARNING | wxYES_NO);
    if (res == wxYES) {
        //FRAME_WINDOW->DoClientCommand(wxT("delvdesc"), itemData->vulnID);
        DrawTree();
    }
}

void wiVulnDB::OnSaveObj( wxCommandEvent& event )
{
    SaveVulner();
   	wxTreeItemId itemID = m_treeVDB->GetSelection();
	if ( itemID.IsOk() ) {
	    m_treeVDB->SetItemBold(itemID);
	}

    DrawTree();
}

void wiVulnDB::OnVulnerSelect( wxTreeEvent& event )
{
    wxTreeItemId idTree;
    wiVulnTreeData* itemData;
    wxTreeItemIdValue cookie;
    wxString vulnDesc;
    wxString content;
    wxXmlDocument doc;
    long idx;

    if (m_forceSelect) {
        return;
    }
    if ( !ContinueUnsaved() ) {
        event.Veto();
        return;
    }
    idTree = event.GetItem();
    if (!idTree.IsOk()) {
        event.Veto();
        return;
    }
    if (event.GetOldItem().IsOk()) {
        m_treeVDB->SetItemBold(event.GetOldItem(), false);
    }
    itemData = (wiVulnTreeData*)m_treeVDB->GetItemData(idTree);
    if (itemData) {
        if ( itemData->vulnID.IsEmpty() ) {
            idTree = m_treeVDB->GetFirstChild(idTree, cookie);
            if (idTree.IsOk()) {
                itemData = (wiVulnTreeData*)m_treeVDB->GetItemData(idTree);
                if (!itemData || itemData->vulnID.IsEmpty() ) {
                    event.Veto();
                    return;
                }
            }
            else {
                // no childs
                m_txtLabel->Clear();
                m_chSeverity->Select(0);
                m_txtTiltle->Clear();
                m_txtShort->Clear();
                m_scDesc->ClearAll();
                m_bDataChanged = false;
                return;
            }
        }
        //vulnDesc = FRAME_WINDOW->DoClientCommand(wxT("getvdesc"), itemData->vulnID);
        vulnDesc = vdescXML;
        wxCharBuffer buff = vulnDesc.utf8_str();
        int buffLen = strlen(buff.data());
        wxMemoryInputStream xmlStream(buff.data(), buffLen);
        //wxLogNull logger;
        if ( doc.Load(xmlStream) ) {
            wxXmlNode *root, *chld;
            root = doc.GetRoot();
            if (root != NULL) {
                m_txtLabel->SetValue(itemData->vulnID);
                chld = root->GetChildren();
                while (chld != NULL) {
                    if (chld->GetName() == wxT("title")) {
                        content = chld->GetNodeContent();
                        m_txtTiltle->SetValue(UnscreenXML(content));
                    }
                    if (chld->GetName() == wxT("severity")) {
                        content = chld->GetNodeContent();
                        if (! content.ToLong(&idx) ) {
                            idx = 0;
                        }
                        if (idx < 0 || idx > 5) {
                            idx = 0;
                        }
                        m_chSeverity->Select(idx);
                    }
                    if (chld->GetName() == wxT("short_desc")) {
                        content = chld->GetNodeContent();
                        m_txtShort->SetValue(UnscreenXML(content));
                    }
                    if (chld->GetName() == wxT("description")) {
                        content = chld->GetNodeContent();
                        m_scDesc->SetText(UnscreenXML(content));
                    }

                    chld = chld->GetNext();
                }
                m_treeVDB->SetItemBold(idTree, false);
                m_bDataChanged = false;
            }
            else {
                event.Veto();
            }
        }
        else {
            event.Veto();
        }
    }
    else {
        event.Veto();
    }
}

void wiVulnDB::OnDataChanged( wxCommandEvent& event )
{
	m_bDataChanged = true;
	m_toolBar->EnableTool(wxID_SAVE, true);
	wxTreeItemId itemID = m_treeVDB->GetSelection();
	if ( itemID.IsOk() ) {
	    m_treeVDB->SetItemBold(itemID);
	}
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
            SaveVulner();
        }
    }
    return true;
}

void wiVulnDB::DrawTree()
{
    wxWindowUpdateLocker noUpdates(m_treeVDB);
    wiVulnerIdHash hashMap;
    wiVulnerIdHash::iterator  it;
    wxTreeItemId parent, rootID, itemID;
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
                            wiVulnTreeData* itemData = new wiVulnTreeData;
                            itemData->vulnID = wxEmptyString;
                            itemData->pluginID = plgName;
                            m_treeVDB->SetItemData(parent, itemData);
                        }
                        else {
                            parent = it->second;
                        }
                        strLev = wxString::Format(wxT("%s: %s"), strId.c_str(), strTitle.c_str());
                        itemID = m_treeVDB->AppendItem(parent, strLev, severity);
                        wiVulnTreeData* itemData = new wiVulnTreeData;
                        itemData->vulnID = strId;
                        itemData->pluginID = plgName;
                        m_treeVDB->SetItemData(itemID, itemData);
                    }
                    chld = chld->GetNext();
                } // end child loop
            } // end if root presented
        } // end if doc loaded
    }
}

void wiVulnDB::SaveVulner()
{
    wxString res = ScreenXML(m_scDesc->GetText());
    //FRAME_WINDOW->DoClientCommand(wxT("setvdesc"), res);
}

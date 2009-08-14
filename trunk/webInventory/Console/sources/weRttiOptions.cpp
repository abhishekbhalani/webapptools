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
*/
/***************************************************************
 * @file      weRttiOptions.cpp
 * @brief     Defines wiTreeData Class for report tree
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      10.08.2009
 **************************************************************/
#include <wx/log.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/spinctrl.h>
#include "weRttiOptions.h"
#include "optionnames.h"

weRttiOptions::weRttiOptions()
{
    //ctor
}

wxXmlNode* weRttiOptions::GetOptions(wxWindow* panel, wxXmlNode* root, bool verbose)
{
    wxWindowList children;
    wxWindowList children2;
    wxWindowListNode *node;
    wxWindow* chld;
    wxString name, type;
    wxString className;
    long lType;
    size_t spos, epos;
    wxString sData;
    long iData;
    double dData;
    bool bData;

    if (root == NULL) {
        root = new wxXmlNode(wxXML_ELEMENT_NODE, wxT("options"));
    }

    children = panel->GetChildren();
    node = children.GetFirst();
    while(node) {
        chld = node->GetData();
        name = chld->GetName();
        className = chld->GetClassInfo()->GetClassName();
        type = wxT("8"); // default - string

        spos = name.Find(wxT('<'));
        epos = name.Find(wxT('>'), true);
        if(spos != wxNOT_FOUND) {
            if (epos == wxNOT_FOUND) {
                type = name.Mid(spos + 1, wxSTRING_MAXLEN);
            }
            else {
                type = name.Mid(spos + 1, epos - spos -1);
            }
            name = name.Mid(0, spos);
        }
        type.ToLong(&lType);

        if (lType >= 0 && lType < 9) {
            if (className == wxT("wxTextCtrl"))
            {
                sData = ((wxTextCtrl*)chld)->GetValue();
                if (lType < 2) {
                    // char and uchar value
                    SaveTaskOption(root, name, type, sData);
                }
                if (lType > 1 && lType < 6) {
                    // all int values
                    sData.ToLong(&iData);
                    SaveTaskOptionInt(root, name, iData);
                }
                if (lType == 6) {
                    // bool values
                    sData.ToLong(&iData);
                    SaveTaskOptionBool(root, name, iData);
                }
                if (lType == 7) {
                    // double values
                    sData.ToDouble(&dData);
                    //SaveTaskOptionDouble(root, name, dData);
                }
                if (lType == 8) {
                    // string values
                    SaveTaskOption(root, name, type, sData);
                }
            }
            else if (className == wxT("wxRadioButton"))
            {
                bData = ((wxRadioButton*)chld)->GetValue();
                SaveTaskOptionBool(root, name, bData);
            }
            else if (className == wxT("wxCheckBox"))
            {
                bData = ((wxCheckBox*)chld)->GetValue();
                SaveTaskOptionBool(root, name, bData);
            }
            else if (className == wxT("wxSpinCtrl"))
            {
                iData = ((wxSpinCtrl*)chld)->GetValue();
                SaveTaskOptionInt(root, name, iData);
            }
            else if (className == wxT("wxChoice"))
            {
                iData = ((wxChoice*)chld)->GetCurrentSelection();
                if (lType == 8) {
                    sData = ((wxChoice*)chld)->GetString(iData);
                    SaveTaskOption(root, name, type, sData);
                }
                else {
                    SaveTaskOptionInt(root, name, iData);
                }
            }
            else if (className == wxT("wxComboBox"))
            {
                if (lType == 8) {
                    sData = ((wxComboBox*)chld)->GetValue();
                    SaveTaskOption(root, name, type, sData);
                }
                else {
                    iData = ((wxComboBox*)chld)->GetCurrentSelection();
                    SaveTaskOptionInt(root, name, iData);
                }
            }
            else
            {
                if (verbose) {
                    ::wxLogError(_("Unknown widget class: %s (type=%d, name='%s'"), className.c_str(), lType, name.c_str());
                };
            }
        }
        else {
            if (verbose) {
                wxLogError(_("Unknown option type: %d (class=%s, name='%s'"), lType, className.c_str(), name.c_str());
            }
        }

        children2 = chld->GetChildren();
        if (children2.GetCount() > 0) {
            GetOptions(chld, root, verbose);
        }
        node = node->GetNext();
    }
    return root;
}

void weRttiOptions::SaveTaskOption (wxXmlNode *root, const wxString& name, const wxString& type, const wxString& value)
{
    wxXmlNode *chld, *content;

    chld = new wxXmlNode(wxXML_ELEMENT_NODE, wxT("option"));
    chld->AddProperty(wxT("name"), name);
    chld->AddProperty(wxT("type"), type);
    content = new wxXmlNode(wxXML_TEXT_NODE, wxT(""));
    content->SetContent(value);
    chld->AddChild(content);
    root->AddChild(chld);
}

void weRttiOptions::SaveTaskOptionBool (wxXmlNode *root, const wxString& name, const bool& value)
{
    wxString content = wxT("0");
    if (value) {
        content = wxT("1");
    }
    SaveTaskOption(root, name, wxT(weoTypeBool), content);
}

void weRttiOptions::SaveTaskOptionInt (wxXmlNode *root, const wxString& name, const int& value)
{
    wxString content = wxString::Format(wxT("%d"), value);
    SaveTaskOption(root, name, wxT(weoTypeInt), content);
}

void weRttiOptions::SetOptions(wxWindow* panel, wxXmlNode* root, bool verbose /*= false*/)
{
    wxXmlNode* chld;
    wxWindowList children;
    wxWindowListNode *node;
    wxWindow* winchld;
    wxString name, type;
    wxString className;
    wxString value;
    long lType;
    size_t spos, epos;
    long iData;
    double dData;
    bool bData;

    children = panel->GetChildren();
    chld = root->GetChildren();
    while(chld != NULL) {
        wxString winName;
        if(chld->GetType() == wxXML_ELEMENT_NODE) {
            winName = chld->GetPropVal(wxT("name"), wxT(""));
            winName += wxT("<");
            winName += chld->GetPropVal(wxT("type"), wxT("8"));
            winName += wxT(">");
            value = chld->GetNodeContent();

            // search for window name
            winchld = wxWindow::FindWindowByName(winName);
            if (winchld != NULL) {
                // window found
                name = winchld->GetName();
                className = winchld->GetClassInfo()->GetClassName();
                // set control value
                if (className == wxT("wxTextCtrl"))
                {
                    ((wxTextCtrl*)winchld)->SetValue(value);
                }
                else if (className == wxT("wxRadioButton"))
                {
                    value.ToLong(&iData);
                    ((wxRadioButton*)winchld)->SetValue(iData);
                }
                else if (className == wxT("wxCheckBox"))
                {
                    value.ToLong(&iData);
                    ((wxCheckBox*)winchld)->SetValue(iData);
                }
                else if (className == wxT("wxSpinCtrl"))
                {
                    value.ToLong(&iData);
                    ((wxSpinCtrl*)winchld)->SetValue(iData);
                }
                else if (className == wxT("wxChoice"))
                {
                    if (lType == 8) {
                        // todo - set string
                    }
                    else {
                        // todo - set index
                    }
                }
                else if (className == wxT("wxComboBox"))
                {
                    if (lType == 8) {
                        // todo - set string
                    }
                    else {
                        // todo - set index
                    }
                }
                else
                {
                    if (verbose) {
                        ::wxLogError(_("Unknown widget class: %s (type=%d, name='%s'"), className.c_str(), lType, name.c_str());
                    };
                }
            } // end of winchld != NULL
        }
        chld = chld->GetNext();
    }
}

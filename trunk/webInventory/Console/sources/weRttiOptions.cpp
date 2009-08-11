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
    wxString name, type, code;
    long lCode;
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
        type = wxT("8"); // default - string
        code = wxT("-1"); // unknown

        spos = name.Find(wxT('['));
        epos = name.Find(wxT(']'), true);
        if(spos != wxNOT_FOUND) {
            if (epos == wxNOT_FOUND) {
                code = name.Mid(spos + 1, wxSTRING_MAXLEN);
            }
            else {
                code = name.Mid(spos + 1, epos - spos -1);
            }
            name = name.Mid(0, spos);
        }
        code.ToLong(&lCode);

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
            switch(lCode) {
                case TEXT_CTRL:
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
                    break;
                case RADIO_BTN:
                    bData = ((wxRadioButton*)chld)->GetValue();
                    SaveTaskOptionBool(root, name, bData);
                    break;
                case CHECH_BOX:
                    bData = ((wxCheckBox*)chld)->GetValue();
                    SaveTaskOptionBool(root, name, bData);
                    break;
                case SPIN_CTRL:
                    iData = ((wxSpinCtrl*)chld)->GetValue();
                    SaveTaskOptionInt(root, name, iData);
                    break;
                case CHOICE:
                    iData = ((wxChoice*)chld)->GetCurrentSelection();
                    if (lType == 8) {
                        sData = ((wxChoice*)chld)->GetString(iData);
                        SaveTaskOption(root, name, type, sData);
                    }
                    else {
                        SaveTaskOptionInt(root, name, iData);
                    }
                    break;
                case COMBOBOX:
                    if (lType == 8) {
                        sData = ((wxComboBox*)chld)->GetValue();
                        SaveTaskOption(root, name, type, sData);
                    }
                    else {
                        iData = ((wxComboBox*)chld)->GetCurrentSelection();
                        SaveTaskOptionInt(root, name, iData);
                    }
                    break;
                default:
                    if (verbose) {
                        ::wxLogError(_("Unknown widget code: %d (type=%d, name='%s'"), lCode, lType, name.c_str());
                    };
            };
        }
        else {
            if (verbose) {
                wxLogError(_("Unknown option type: %d (code=%d, name='%s'"), lType, lCode, name.c_str());
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


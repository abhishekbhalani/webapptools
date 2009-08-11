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
 * @file      weRttiOptions.h
 * @brief     Defines wiTreeData Class for report tree
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      10.08.2009
 **************************************************************/
#ifndef WERTTIOPTIONS_H
#define WERTTIOPTIONS_H

#include <wx/window.h>
#include <wx/xml/xml.h>

#define UNKNOWN         -1
#define TEXT_CTRL       0
#define RADIO_BTN       1
#define CHECH_BOX       2
#define SPIN_CTRL       3
#define CHOICE          4
#define COMBOBOX        5

class weRttiOptions
{
    public:
        weRttiOptions();

        static wxXmlNode* GetOptions(wxWindow* panel, wxXmlNode* root, bool verbose = false);
        static void SaveTaskOption (wxXmlNode *root, const wxString& name, const wxString& type, const wxString& value);
        static void SaveTaskOptionBool (wxXmlNode *root, const wxString& name, const bool& value);
        static void SaveTaskOptionInt (wxXmlNode *root, const wxString& name, const int& value);

    protected:
    private:
};

#endif // WERTTIOPTIONS_H

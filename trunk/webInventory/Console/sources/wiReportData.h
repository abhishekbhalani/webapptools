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
 * @file      wiReportData.h
 * @brief     Declaration of the Report classes
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      06.08.2009
 **************************************************************/
#ifndef WIREPORTDATA_H
#define WIREPORTDATA_H

#include <wx/richtext/richtextctrl.h>

class wiReportData
{
    public:
        wiReportData() {};
        virtual ~wiReportData() {};

        virtual void WriteReport(wxRichTextCtrl& rt) = 0;
};

class wiSimpleReport : public wiReportData
{
    public:
        wiSimpleReport(wxString msg = wxT(""), int lvl = 0);
        virtual ~wiSimpleReport();

        virtual void WriteReport(wxRichTextCtrl& rt);

        void SetText(wxString msg = wxT("")) { text = msg; };
        void SetLevel(int lvl = 0) { level = lvl; };
    protected:
        wxString text;
        int      level;
};

#endif // WIREPORTDATA_H

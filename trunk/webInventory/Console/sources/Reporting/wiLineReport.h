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
 * @file      wiLineReport.h
 * @brief     Declares wiLineReport class
 * @author    Andrew "Stinger" Abramov (stinger@lab18.net)
 * @date      31.08.2009
 **************************************************************/
#ifndef WIONELINEREPORT_H
#define WIONELINEREPORT_H

#include "wiReportData.h"


class wiLineReport : public wiReportData
{
    public:
        wiLineReport(wxString msg = wxT(""), int lvl = 0);
        virtual ~wiLineReport();

        virtual void  WriteReport(wxRichTextCtrl& rt);
    protected:
        wxString text;
        int level;
};

#endif // WIONELINEREPORT_H

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
 * @file      wiLineReport.cpp
 * @brief     Defines wiLineReport class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      31.08.2009
 **************************************************************/
#include "wiLineReport.h"

wiLineReport::wiLineReport(wxString msg /*= wxT("")*/, int lvl /*= 0*/)
{
    text = msg;
    level = lvl;
    LoadTemplate(wxT("linereport.xml"));
}

wiLineReport::~wiLineReport()
{
    //dtor
}

void wiLineReport::WriteReport(wxRichTextCtrl& rt)
{
    wxString style;

    rt.SetInsertionPointEnd();
    style = wxString::Format(wxT("level%d"), level);
    AddFormattedText(style, text + wxT("\n"), rt);
}

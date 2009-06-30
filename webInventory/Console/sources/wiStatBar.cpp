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
 * @file      wiStatBar.cpp
 * @brief     Code for wiStatBar class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      30.06.2009
 **************************************************************/
#include "wiStatBar.h"
#include "../images/tree_no.xpm"
#include "../images/tree_unk.xpm"
#include "../images/tree_yes.xpm"

BEGIN_EVENT_TABLE(wiStatBar, wxStatusBar)
    EVT_SIZE(wiStatBar::OnSize)
END_EVENT_TABLE()

wiStatBar::wiStatBar(wxWindow *parent) : wxStatusBar(parent, wxID_ANY)
{
    static const int widths[4] = { 16,  100, 200, -1 };
    SetFieldsCount(4);
    SetStatusWidths(4, widths);
    m_statbmp = new wxStaticBitmap(this, wxID_ANY, wxIcon(tree_unk));
    SetMinHeight(16);
}

wiStatBar::~wiStatBar()
{
}

void wiStatBar::OnSize(wxSizeEvent& event)
{
    wxRect rect;

    GetFieldRect(0, rect);
    wxSize size = m_statbmp->GetSize();

    m_statbmp->Move(rect.x + (rect.width - size.x) / 2,
        rect.y + (rect.height - size.y) / 2);

    event.Skip();
}


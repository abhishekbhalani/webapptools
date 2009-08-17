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
#if defined(__WXMSW__)
    static const int widths[4] = { 16,  120, 220, -1 };
#elif defined(__UNIX__)
    static const int widths[4] = { 22,  120, 220, -1 };
#else
    static const int widths[4] = { 16,  120, 220, -1 };
#endif
    SetFieldsCount(4);
    SetStatusWidths(4, widths);
    m_statbmp = NULL;
    SetImage(wiSTATUS_BAR_UNK);
}

wiStatBar::~wiStatBar()
{
}

void wiStatBar::SetImage(int status)
{
    if (m_statbmp) {
        delete m_statbmp;
    }
    switch(status) {
        case wiSTATUS_BAR_NO:
            m_statbmp = new wxStaticBitmap(this, wxID_ANY, wxIcon(tree_no_xpm));
            break;
        case wiSTATUS_BAR_YES:
            m_statbmp = new wxStaticBitmap(this, wxID_ANY, wxIcon(tree_yes_xpm));
            break;
        default:
            m_statbmp = new wxStaticBitmap(this, wxID_ANY, wxIcon(tree_unk_xpm));
            break;
    };
    SetMinHeight(16);

    wxRect rect;
    GetFieldRect(0, rect);
    wxSize size = m_statbmp->GetSize();

    m_statbmp->Move(rect.x + (rect.width - size.x) / 2,
        rect.y + (rect.height - size.y) / 2);
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


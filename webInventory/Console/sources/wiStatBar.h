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
 * @file      wiStatBar.h
 * @brief     Declaration of the wiStatBar class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      30.06.2009
 **************************************************************/
#ifndef WXSTATBAR_H
#define WXSTATBAR_H

#include <wx/statusbr.h>
#include <wx/statbmp.h>

#define wiSTATUS_BAR_NO     1
#define wiSTATUS_BAR_YES    2
#define wiSTATUS_BAR_UNK    0
/**
 * @class   wiStatBar
 * @brief   A custom status bar which contains controls, icons &c
 */
class wiStatBar : public wxStatusBar
{
public:
    wiStatBar(wxWindow *parent);
    virtual ~wiStatBar();
    void OnSize(wxSizeEvent& event);
    void SetImage(int status);

private:
    wxStaticBitmap *m_statbmp;
    DECLARE_EVENT_TABLE()
};

#endif // WXSTATBAR_H

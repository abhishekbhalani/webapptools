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
 * @file      wiMainForm.cpp
 * @brief     Code for MainForm class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      30.06.2009
 **************************************************************/
#include <wx/filename.h>
#include "wiMainForm.h"
#include "../images/webInventory.xpm"

wiMainForm::wiMainForm( wxWindow* parent ) :
        MainForm( parent ),
        m_cfgEngine(wxT("WebInvent"))
{
    wxInitAllImageHandlers();
    SetIcon(wxICON(mainicon));

    wxString fname;
    wxFileName fPath;

    fname = wxFindFirstFile(wxT("*.mo"), wxFILE);
    while (!fname.IsEmpty()) {
        fPath.Assign(fname);
        fname = fPath.GetName();
        //! @todo: Capitalise first char
        m_chLangs->AppendString(fname);
        fname = wxFindNextFile();
    }
    m_chLangs->SetSelection(0);

    wxString cfgData;
    int iData;
    iData = 0;
    if (m_cfgEngine.Read(wxT("language"), &cfgData)) {
        iData = m_chLangs->FindString(cfgData);
    }
    m_chLangs->SetSelection(iData);

    m_lstActiveTask->InsertColumn(0, wxT(""), wxLIST_FORMAT_LEFT, 18);
    m_lstActiveTask->InsertColumn(1, _("Task name"), wxLIST_FORMAT_LEFT, 300);
    m_lstActiveTask->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, 77);

    m_lstTaskList->InsertColumn(0, wxT(""), wxLIST_FORMAT_LEFT, 18);
    m_lstTaskList->InsertColumn(1, _("Task name"), wxLIST_FORMAT_LEFT, 300);
    m_lstTaskList->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, 77);
}

void wiMainForm::OnClose( wxCloseEvent& event )
{
    // @todo disconnect from server and save state
    m_cfgEngine.Write(wxT("language"), m_chLangs->GetString(m_chLangs->GetSelection()));
    Destroy();
}

void wiMainForm::OnLangChange( wxCommandEvent& event )
{
    m_cfgEngine.Write(wxT("language"), m_chLangs->GetString(m_chLangs->GetSelection()));
}

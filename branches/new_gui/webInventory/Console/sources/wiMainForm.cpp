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
*//************************************************************
 * @file      wiMainForm.cpp
 * @brief     Code for MainForm class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      30.06.2009
 * @date      15.09.2009 - global refactoring
 **************************************************************/
#include "wiMainForm.h"
#include "wiTasks.h"
#include "wiReports.h"
#include "wiSettings.h"

#include "../images/panReports.xpm"
#include "../images/panSettings.xpm"
#include "../images/panTasks.xpm"

wiMainForm::wiMainForm( wxWindow* parent )
:
MainForm( parent )
{
	m_pTasks = new wiTasks( m_mainnb ); //, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mainnb->AddPage( m_pTasks, _("Tasks"), false, wxBitmap( panTasks_xpm ) );
	m_pReports = new wiReports( m_mainnb ); //, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mainnb->AddPage( m_pReports, _("Reports"), false, wxBitmap( panReports_xpm ) );
	m_pSettings = new wiSettings( m_mainnb ); //, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_mainnb->AddPage( m_pSettings, _("Settings"), true, wxBitmap( panSettings_xpm ) );

    m_statusBar = new wiStatBar( this );
    SetStatusBar(m_statusBar);
}

void wiMainForm::OnClose( wxCloseEvent& event )
{
    Destroy();
}

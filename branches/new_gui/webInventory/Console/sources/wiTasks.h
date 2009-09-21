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
 * @file      wiTasks.h
 * @brief     Declaration of the wiTasks class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      15.09.2009
 **************************************************************/
#ifndef __wiTasks__
#define __wiTasks__

#include <wx/treebook.h>
#include "wiGuiData.h"

/** Implementing Tasks */
class wiTasks : public Tasks
{
protected:
    wxTreebook* m_plgBookTree;

	// Handlers for Tasks events.
	void OnAddObject( wxCommandEvent& event );
	void OnEditObject( wxCommandEvent& event );
	void OnDelObject( wxCommandEvent& event );
	void OnTaskKillFocus( wxFocusEvent& event );
	void OnSelectObject( wxListEvent& event );
	void OnChangeProfile( wxCommandEvent& event );
	void OnAddProfile( wxCommandEvent& event );
	void OnCopyProfile( wxCommandEvent& event );
	void OnDelProfile( wxCommandEvent& event );
	void OnTaskApply( wxCommandEvent& event );
	void OnRunTask( wxCommandEvent& event );
	void OnAddPlugin( wxCommandEvent& event );
	void OnRemovePlugin( wxCommandEvent& event );
	void OnPauseTask( wxCommandEvent& event );
	void OnCancelTask( wxCommandEvent& event );
	void OnSortItems( wxListEvent& event );
	void OnTaskSelected( wxListEvent& event );

	// functions
	void OnOptionsPageChanging( wxTreebookEvent& event );

	// members
	wxImageList m_lstImages;

public:
	/** Constructor */
	wiTasks( wxWindow* parent );

	void RebuildTreeView();
	wxPanel* CreateDefaultPanel(wxWindow* parent);

    void Connected(bool forced = true);
    void Disconnected(bool forced = true);
};

#endif // __wiTasks__

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
 * @file      wiMainForm.h
 * @brief     Declaration of the MainForm class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      30.06.2009
 **************************************************************/
#ifndef __wiMainForm__
#define __wiMainForm__

#include "wiGuiData.h"
#include <wx/imaglist.h>
#include <wx/xml/xml.h>
#include "Config.h"
#include "wiTcpClient.h"

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_REPORT_LOADING, -1)
DECLARE_EVENT_TYPE(wxEVT_REPORT_LOAD, -1)
END_DECLARE_EVENT_TYPES()

/**
 * @class   wiMainForm
 * @brief   Implementing MainForm
 */
class wiMainForm : public MainForm
{
public:
	/** Constructor */
	wiMainForm( wxWindow* parent );

	void LoadConnections();
	enum { wxPluginsData = 10000 };

protected:
    // Virtual event handlers, overide them in your derived class
    virtual void OnClose( wxCloseEvent& event );
    virtual void OnConnect( wxCommandEvent& event );
    virtual void OnLangChange( wxCommandEvent& event );
    virtual void OnOptionsPageChanging( wxTreebookEvent& event );
    virtual void OnPlgRefresh( wxCommandEvent& event );
    virtual void OnSortItems( wxListEvent& event );
    virtual void OnStorageChange( wxCommandEvent& event );
    virtual void OnTimer( wxTimerEvent& event );
    virtual void OnSrvStop( wxCommandEvent& event );
    virtual void OnServerLogs( wxCommandEvent& event );

    virtual void OnAddServer( wxCommandEvent& event );
    virtual void OnEditServer( wxCommandEvent& event );
    virtual void OnDelServer( wxCommandEvent& event );

//    virtual void OnAddTask( wxCommandEvent& event );
    virtual void OnDelTask( wxCommandEvent& event );
    virtual void OnRunTask( wxCommandEvent& event );
    virtual void OnCancelTask( wxCommandEvent& event );
    virtual void OnTaskSelected( wxListEvent& event );
    virtual void OnPauseTask( wxCommandEvent& event );

    virtual void OnReportTskFilter( wxCommandEvent& event );
    virtual void OnReportsFilter( wxCommandEvent& event );
    virtual void OnReportsRefresh( wxCommandEvent& event );
    virtual void OnReportExpand( wxTreeEvent& event );
    virtual void OnReportSelected( wxTreeEvent& event );

    virtual void OnAddObject( wxCommandEvent& event );
    virtual void OnEditObject( wxCommandEvent& event );
    virtual void OnDelObject( wxCommandEvent& event );
    virtual void OnSelectObject( wxListEvent& event );

    virtual void OnReportsLoadStart( wxCommandEvent& event );
    virtual void OnReportsLoad( wxCommandEvent& event );

    virtual void OnAddProfile( wxCommandEvent& event );
    virtual void OnCopyProfile( wxCommandEvent& event );
    virtual void OnDelProfile( wxCommandEvent& event );
    virtual void OnTaskApply( wxCommandEvent& event );
    virtual void OnChangeProfile( wxCommandEvent& event );

    virtual void OnAddPlugin( wxCommandEvent& event );
    virtual void OnRemovePlugin( wxCommandEvent& event );

    void ProcessTaskList(const wxString& criteria = wxT(""));
    void ProcessObjects(const wxString& criteria = wxT(""));
    void ProcessProfileList(const wxString& criteria = wxT(""));

    void Disconnected(bool mode = true);
    void Connected(bool mode = true);
    void GetPluginList();
    void SelectTask(int id = -1);

    void FillObjectFilter();
    void GetTaskOptions(const wxString& taskID);
    wxPanel* LoadPluginSettings( PluginInf* plg );
    void RebuildOptionsView();
    void RebuildReportsTree();
    PluginInf* FoundPlugin( const wxString& id);

    CConfigEngine m_cfgEngine;
    wiTcpClient* m_client;
    wxTimer m_timer;
    wxImageList m_lstImages;
    PluginList* m_plugList;
    bool connStatus;
    int m_selectedTask;
    int m_selectedObject;
    int m_selectedActive;
    int m_selectedProf;
    int m_plugins;
};

#endif // __wiMainForm__

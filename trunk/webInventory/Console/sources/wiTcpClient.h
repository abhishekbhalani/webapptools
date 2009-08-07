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
 * @file      wiTcpClient.h
 * @brief     Declaration of the wiTcpClient class
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      02.07.2009
 **************************************************************/
#ifndef __WITCPCLIENT_H__
#define __WITCPCLIENT_H__

#include <wx/string.h>

#include "messages.h"

class wiInternalTcp;

class wiTcpClient
{
public:
    wiTcpClient(const char* host, const char* port);
    ~wiTcpClient();

    bool Connect();
    bool Ping();
    wxString GetScannerVersion();
    wxString DoCmd(const wxString& cmd, const wxString& payload);
    const wxString& GetLastError() { return lastError; };
    TaskList* GetTaskList(const wxString& criteria = wxT(""));
    PluginList* GetPluginList(const wxString& criteria = wxT(""));
    ScanList* GetScanList(const wxString& criteria = wxT(""));
    ObjectList* GetObjectList(const wxString& criteria = wxT(""));
    wxString UpdateObject(ObjectInfo& objInfo);

protected:
    wiInternalTcp* client;
    wxString lastError;

private:
};

#endif // __WITCPCLIENT_H__

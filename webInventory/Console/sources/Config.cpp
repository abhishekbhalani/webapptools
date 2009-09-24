/*
 * iConfig.cxx
 * Copyright (C) Andrew Abramov 2007 <stinger@lab18.net>
 *
 * I-Gate is free software. But sources of main part are closed.
 * Plugins for iGate are in open-source domain.
 *
 * You may redistribute it but not modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * main.cc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with main.cc.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include <wx/wx.h>
#include <wx/config.h>

#include "Config.h"

///////////////////////////////////////////////////////////////////////////

CConfigEngine::CConfigEngine(const wxString& appName)
{
    m_appName = appName;
    wxTheApp->SetAppName(m_appName);
    wxTheApp->SetVendorName(APP_VENDOR);
#if defined(__WXMSW__)
    wxGetEnv(wxT("APPDATA"), &m_localDir);
    config = new wxConfig(m_appName);
    m_dirSlash = wxT("\\");
#else
    wxGetEnv(wxT("HOME"), &m_localDir);
    config = new wxConfig();
    m_dirSlash = wxT("/");
#endif
    m_localDir += m_dirSlash + wxT(".") + m_appName + wxT(".cfg") + m_dirSlash;
	if (!wxDirExists(m_localDir))
	{
		wxMkdir(m_localDir, 0755);
	}
}

CConfigEngine::~CConfigEngine()
{
    if (config)
        config->Flush();
}

void CConfigEngine::Flush()
{
    if (config)
        config->Flush();
}

void CConfigEngine::Refresh()
{
    wxConfigBase *cfg;

    if (config) {
        cfg = config;
    #if defined(__WXMSW__)
        config = new wxConfig(m_appName);
    #else
        config = new wxConfig();
    #endif
        delete cfg;
        config->Flush();
    }
}

bool CConfigEngine::Read(wxString param, wxString *data)
{
    if (config) {
        return config->Read(param, data);
    }
    return false;
}

bool CConfigEngine::Read(wxString param, int *data)
{
    if (config) {
        return config->Read(param, data);
    }
    return false;
}

bool CConfigEngine::Read(wxString param, bool *data)
{
    if (config) {
        return config->Read(param, data);
    }
    return false;
}

void CConfigEngine::Write(wxString param, wxString data)
{
    if (config) {
        config->Write(param, data);
    }
}

void CConfigEngine::Write(wxString param, int data)
{
    if (config) {
        config->Write(param, data);
    }
}

void CConfigEngine::Write(wxString param, bool data)
{
    if (config) {
        config->Write(param, data);
    }
}

wxString CConfigEngine::GetAccountName(int idx)
{
    wxString res;

    res.Empty();
    if (idx > -1){
        wxString data;
        if (Read(wxString::Format(wxT("Connection%d/Name"), idx), &data)) {
            res = data;
        }
    }
    return res;
}

int CConfigEngine::GetAccountIndex(wxString name)
{
    int res = -1;
    int idx = 0;
    wxString data;

    while (Read(wxString::Format(wxT("Connection%d/Name"), idx), &data)) {
        if (data == name) {
            res = idx;
            break;
        }
        idx++;
    }
    return res;
}

void CConfigEngine::CopyAccount(int from, int to)
{
    wxString data;
    int number;

    Read(wxString::Format(wxT("Connection%d/Name"), from), &data);
    Write(wxString::Format(wxT("Connection%d/Name"), to), data);
    if (Read(wxString::Format(wxT("Connection%d/Host"), from), &data)) {
        Write(wxString::Format(wxT("Connection%d/Host"), to), data);
    }
    if (Read(wxString::Format(wxT("Connection%d/Port"), from), &number)) {
        Write(wxString::Format(wxT("Connection%d/Port"), to), number);
    }
}

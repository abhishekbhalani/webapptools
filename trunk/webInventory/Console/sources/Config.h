/*
 * This program is free software; you can redistribute it but not modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef ICONFIG_H_INCLUDED
#define ICONFIG_H_INCLUDED

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/confbase.h>

class CConfigEngine {
    private:

    protected:
        wxConfigBase *config;
        wxString m_dirSlash;
        wxString m_localDir;
        wxString m_appName;

    public:
        CConfigEngine(const wxString& appName);
        ~CConfigEngine();

        void Flush();
        void Refresh();
        wxString GetLocalDir(){return m_localDir;};
        wxString GetAccountName(int idx);
        wxString GetDirSlash(){return m_dirSlash;};
        int GetAccountIndex(wxString name);
        void CopyAccount(int from, int to);
        bool DeleteGroup(wxString name){return config->DeleteGroup(name);}
        bool DeleteEntry(wxString name){return config->DeleteEntry(name);}

        bool Read(wxString param, wxString *data);
        bool Read(wxString param, int *data);
        bool Read(wxString param, bool *data);
        void Write(wxString param, wxString data);
        void Write(wxString param, int data);
        void Write(wxString param, bool data);
};

#endif // ICONFIG_H_INCLUDED

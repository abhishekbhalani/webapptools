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
 * @file      weDataHandler.cpp
 * @brief     "data:" protocol for HTML pages
 * @author    Tursas Pohielainen (tursasp@gmail.com)
 * @date      02.10.2009
 **************************************************************/
#include "weDataHandler.h"
#include <wx/mstream.h>

static char lookupTable[96] = {
    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3e, 0x7f, 0x3e, 0x7f, 0x3f, // 0x20
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x7f, 0x7f, 0x7f, 0x00, 0x7f, 0x7f, // 0x30
    0x7f, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, // 0x40
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x7f, 0x7f, 0x7f, 0x7f, 0x3f, // 0x50
    0x7f, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, // 0x60
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f  // 0x70
};

static unsigned char* Base64Decode(unsigned char* source, int len, int *decLen)
{
    unsigned char*  result = new unsigned char[len];
    unsigned char*  cDec;
    unsigned char   buff[3], smb;
    int     idx, padd, cdPos;

    if (source == NULL || decLen == NULL) {
        return NULL;
    }

    idx = 0;
    *decLen = 0;
    cDec = result;
    cdPos = 0;
    memset(buff, 0, 3);
    padd = 3;
    while(len > 0) {
        if(*source > 0x20 && *source < 0x80) {
            smb = lookupTable[*source - 0x20];
            if (smb != 0x7f) {
                if (idx == 0) {
                    buff[0] = smb << 2;
                }
                else if (idx == 1) {
                    buff[0] += smb >> 4;
                    buff[1] = (smb & 0x0f) << 4;
                }
                else if (idx == 2) {
                    buff[1] += smb >> 2;
                    buff[2] = (smb & 0x03) << 6;
                }
                else if (idx == 3) {
                    buff[2] += smb;
                }
                idx++;
                if (*source == '=') {
                    padd--;
                }
                if (idx == 4) {
                    idx = 0;
                    memmove(cDec, buff, padd);
                    cDec += padd;
                    *decLen += padd;
                    padd = 3;
                }
            }
        }
        len--;
        source++;
    }
    if (idx > 0) {
        idx = 0;
        memmove(cDec, buff, padd);
        padd = 3;
        cDec += 3;
        cDec += padd;
        *decLen += padd;
    }
    return result;
}

class wiMemoryInputStream : public wxMemoryInputStream {
    protected:
        unsigned char* buffer;
    public:
        wiMemoryInputStream(unsigned char* buff, int buffLen);
        virtual ~wiMemoryInputStream();
};

wiMemoryInputStream::wiMemoryInputStream(unsigned char* buff, int buffLen) :
wxMemoryInputStream(buff, buffLen)
{
    buffer = buff;
}

wiMemoryInputStream::~wiMemoryInputStream()
{
    if (buffer) {
        delete buffer;
    }
}

weDataHandler::weDataHandler()
{
    //ctor
}

bool weDataHandler::CanOpen(const wxString& location)
{
    return (GetProtocol(location) == wxT("data"));
}

wxFSFile* weDataHandler::OpenFile(wxFileSystem& fs, const wxString& location)
{
    wxFSFile    *result = NULL;
    wxDateTime  dtModif = wxDateTime::Now();
    wxString    docMain, docType;
    int         binLen;
    int         pos;
    unsigned char* binary;

    docMain = GetRightLocation(location);
    if (docMain.IsEmpty()) {
        return result;
    }

    docType = wxT("application/octet-stream");
    pos = docMain.Find(wxT(';'));
    if (pos != wxNOT_FOUND) {
        docType = docMain.Mid(0, pos);
    }

    pos = docMain.Find(wxT("base64,"));
    if (pos != wxNOT_FOUND) {
        docMain = docMain.Mid(pos + 7); // after "base64,"
        wxCharBuffer chBuff = docMain.ToAscii();
        binary = Base64Decode((unsigned char*)chBuff.data(), docMain.Len(), &binLen);
        if (binary != NULL && binLen != 0) {
            result = new wxFSFile(new wiMemoryInputStream(binary, binLen), location, docType, wxEmptyString, dtModif);
            //delete binary;
        }
    }
    return result; //new wxFSFile;
}

wxString weDataHandler::GetMimeTypeFromExt(const wxString& location)
{
    wxString    result;
    wxString    docMain;
    int         pos;

    docMain = GetLeftLocation(location);
    result = wxT("application/octet-stream");
    pos = docMain.Find(wxT(';'));
    if (pos != wxNOT_FOUND) {
        result = docMain.Mid(0, pos);
    }
    return result;
}

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
 * @file      weDataHandler.h
 * @brief     "data:" protocol for HTML pages
 * @author    Tursas Pohielainen (tursasp@gmail.com)
 * @date      02.10.2009
 **************************************************************/
#ifndef WEDATAHANDLER_H
#define WEDATAHANDLER_H

#include "wx/filesys.h"


class weDataHandler : public wxFileSystemHandler
{
    public:
        weDataHandler();

        // wxFileSystem implementation
        virtual bool CanOpen(const wxString& location);
        virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location);
        virtual wxString GetMimeTypeFromExt(const wxString& location);
    protected:
    private:
};

#endif // WEDATAHANDLER_H

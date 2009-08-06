/*
    scanServer is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of scanServer

    scanServer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    scanServer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with inventoryScanner.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __WEPROFILE_H__
#define __WEPROFILE_H__
#include <weOptions.h>
#include <weTagScanner.h>

using namespace boost;

class weProfile : public iweOptionsProvider
{
public:
    string ToXml( void );
    void FromXml( string input );
    void FromXml( WeTagScanner& sc, int token = -1 );
};

#endif //__WEPROFILE_H__

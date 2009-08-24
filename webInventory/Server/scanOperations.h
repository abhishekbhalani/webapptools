/*
    inventoryScanner is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of inventoryScanner

    inventoryScanner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    inventoryScanneris distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with inventoryScanner.  If not, see <http://www.gnu.org/licenses/>.
*//************************************************************
 * @file   scanOperations.h
 *
 * @brief  Operations to process scan-related queries 
 *
 * This is the sample implementation and the subject to change
 * @author	Aabramov
 * @date	05.08.2009
 **************************************************************/
#ifndef __SCANOPERATIONS_H__
#define __SCANOPERATIONS_H__
#include "messages.h"

extern ScanList* get_scan_list(const string& criteria = "");

#endif //__SCANOPERATIONS_H__

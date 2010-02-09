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
 * @file      treeData.h
 * @brief     Defines wiTreeData Class for report tree
 * @author    Andrew "Stinger" Abramov (stinger911@gmail.com)
 * @date      05.08.2009
 **************************************************************/

#ifndef WITREEDATA_H
#define WITREEDATA_H

#include <wx/treectrl.h>

#define WI_TREE_NODE_NONE       -1
#define WI_TREE_NODE_OBJECT     0
#define WI_TREE_NODE_SCAN       1
#define WI_TREE_NODE_SCANDATA   2
#define WI_TREE_NODE_SCANVULNER 3

class wiReportData;

class wiTreeData: public wxTreeItemData
{
    public:
        wiTreeData()
        {
            nodeType = WI_TREE_NODE_NONE;
            objectID = -1;
            hasData = NULL;
        };

        // treeData information
        wiReportData* hasData;
        int nodeType;
        int objectID;
};

class wiPlgTreeData: public wxTreeItemData
{
    public:
        wiPlgTreeData(PluginInf* _plg)
        {
            plg = _plg;
        };

        // treeData information
        PluginInf* plg;
};

#endif // WITREEDATA_H

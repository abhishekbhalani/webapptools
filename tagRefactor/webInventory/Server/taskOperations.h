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
 * @file   taskOperations.cpp
 *
 * @brief  Operations to process task-related queries 
 *
 * This is the sample implementation and the subject to change
 * @author	Aabramov
 * @date	03.07.2009
 **************************************************************/
#ifndef __TASKOPERATIONS_H__
#define __TASKOPERATIONS_H__
#include "messages.h"

extern string taskDbDir;

extern TaskList* get_task_list(const string& criteria = "");
extern string add_task(const string& name);
extern bool del_task(const string& id);
extern bool run_task(const string& id);
extern bool pause_task(const string& id);
extern bool cancel_task(const string& id);
extern string get_task_opts (const string& id);
extern bool set_task_opts (const string& xml);

extern ObjectList* get_object_list(const string& criteria = "");
extern string update_object(ObjectInf& obj);
extern bool del_object(const string& id);

extern string get_profile_opts (const string& id);
extern bool set_profile_opts (const string& xml);
extern bool del_profile (const string& id);
extern ProfileList* get_profile_list(const string& criteria = "");

extern string get_tsk_log(const string& tskid, int tail);

#endif //__TASKOPERATIONS_H__

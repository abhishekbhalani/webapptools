/*
    tskScanner is the web-application audit program
    Copyright (C) 2009 Andrew "Stinger" Abramov stinger911@gmail.com

    This file is part of tskScanner

    tskScanner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    tskScanner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with tskScanner.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__
#include <weHelper.h>
#include <weDispatch.h>
#include <weTask.h>
#include <boost/thread.hpp>

class locked_data
{
public:
    locked_data();
    ~locked_data();
    void destroy();

    WeTask* load_task(const string& id);
    void save_task( void );
    bool check_state();

    boost::mutex    locker;
    bool            execution;
    bool            pause;
    boost::condition_variable cond;

    WeDispatch* dispatcher;
    WeTask*     task_info;
};

extern locked_data  globalData;

void watch_dog_thread(const string& id);

#endif //__WATCHDOG_H__

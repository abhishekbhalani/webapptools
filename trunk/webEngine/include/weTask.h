/*
    webEngine is the HTML processing library
    Copyright (C) 2009 Andrew Abramov aabramov@ptsecurity.ru

    This file is part of webEngine

    webEngine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    webEngine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with webEngine.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __WETASK_H__
#define __WETASK_H__

#include <string>
#include <map>
#include <boost/any.hpp>
#include "weiBase.h"
#include "weOptions.h"
#include "weiTransport.h"

using boost::any_cast;
using namespace boost;
using namespace std;

#define WE_VERSION_ID   0

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeTask
///
/// @brief  Entry point to execute any actions with webEngine
///
/// @author A. Abramov
/// @date   09.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeTask : public iweOptionsProvider
{
public:
    WeTask();
    WeTask(WeTask& cpy);
    ~WeTask();

    void AddTransport(const string& transp);
    void AddTransport(iweTransport* transp);

    bool IsReady();
    iweResponse* GetRequest(iweRequest* req);
    void GetRequestAsync(iweRequest* req);

    string ToXml( void );
    void FromXml( string input );
    void FromXml( WeTagScanner& sc, int token = -1 );

    void WaitForData();
    void CalcStatus();

#ifndef __DOXYGEN__
protected:
    typedef map<string, iweRequest*> WeRequestMap;
    vector<iweTransport*>   transports;
    bool processThread;
    void* mutex_ptr;
    void* event_ptr;
    int taskQueueSize;
    WeRequestMap taskList;
    WeResponseList taskQueue;
#endif //__DOXYGEN__

private:
    DECLARE_SERIALIZATOR
    {
        ar & BOOST_SERIALIZATION_NVP(options);
    };
    friend void WeTaskProcessor(WeTask* tsk);
};

BOOST_CLASS_TRACKING(WeTask, boost::serialization::track_never)

#endif //__WETASK_H__

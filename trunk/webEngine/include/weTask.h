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
#include "weiParser.h"

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

    virtual WeOption& Option(const string& name);
    virtual bool IsSet(const string& name);
    virtual void Option(const string& name, WeOptionVal val);

    void SetTransport(const string& transp);
    void SetTransport(iweTransport* transp);

    bool IsReady();
    iweResponse* GetRequest(iweRequest* req);
    iweResponse* GetRequestAsync(iweRequest* req);

#ifndef __DOXYGEN__
protected:
    WeOptions       options;
    iweTransport*   transport;
#endif //__DOXYGEN__

private:
    DECLARE_SERIALIZATOR
    {
        ar & BOOST_SERIALIZATION_NVP(options);
    };
};

BOOST_CLASS_TRACKING(WeTask, boost::serialization::track_never)

#endif //__WETASK_H__

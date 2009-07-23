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
*/
#include "server.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <strstream>
#include <weiBase.h>
#include <weLogger.h>
#include <weDispatch.h>
#include "messages.h"
#include "version.h"
#include "taskOperations.h"

extern WeDispatch* globalDispatcher;
string get_plugin_list(string filter);
string get_plugin_ui(string filter);

int process_message(boost::asio::streambuf* buff, size_t bufSize, session* sess)
{
    int retval = 0; // close connection
    bool processed = false;
    Message msg;

    try
    {
        buff->commit(bufSize);
        istream data(buff);
        boost::archive::xml_iarchive ia(data);
        ia >> BOOST_SERIALIZATION_NVP(msg);
        LOG4CXX_TRACE(WeLogger::GetLogger(), "[" << sess->socket().remote_endpoint().address() << "] process_message: " <<  msg.cmd);
        retval = 0;
        sess->last = posix_time::second_clock::local_time();

        // !!! DEBUG fake_task_processing 
        fake_task_processing();

        //////////////////////////////////////////////////////////////////////////
        // EXIT command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "exit"))
        {
            if (globalDispatcher->Storage() != NULL)
            {
                globalDispatcher->Storage()->Flush();
            }
            retval = -1;
            processed = true;
        }
        //////////////////////////////////////////////////////////////////////////
        // CLOSE command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "close"))
        {
            if (globalDispatcher->Storage() != NULL)
            {
                globalDispatcher->Storage()->Flush();
            }
            retval = 0;
            processed = true;
        }
        //////////////////////////////////////////////////////////////////////////
        // VERSION command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "version"))
        {
            LOG4CXX_INFO(WeLogger::GetLogger(), "[" << sess->socket().remote_endpoint().address() << "] version requested");
            msg.data = AutoVersion::FULLVERSION_STRING;
            msg.data += " ";
            msg.data += AutoVersion::STATUS;
#ifdef _WIN32_WINNT
            msg.data += " (Windows)";
#else
            msg.data += " (Linux)";
#endif
            retval = 1;
            processed = true;
        }
        //////////////////////////////////////////////////////////////////////////
        // PING command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "ping"))
        {
            msg.cmd = "pong";
            msg.data = posix_time::to_simple_string(sess->last);
            retval = 1;
            processed = true;
        }
        //////////////////////////////////////////////////////////////////////////
        // TASKS command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "tasks"))
        {
            msg.cmd = "tasks";
            TaskList *lst;

            lst = get_task_list(msg.data);

            if (lst != NULL)
            {
                { // auto-destroy block for output stream
                    ostrstream oss;
                    { // auto-destroy block for xml_oarchive
                        boost::archive::xml_oarchive oa(oss);
                        oa << BOOST_SERIALIZATION_NVP(lst);
                    }
                    msg.data = string(oss.str(), oss.rdbuf()->pcount());
                    delete lst;
                }
            }
            else {
                msg.data = "";
            }
            retval = 1;
            processed = true;
        }
        //////////////////////////////////////////////////////////////////////////
        // ADDTASK command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "addtask"))
        {
            msg.data = add_task(msg.data);
            retval = 1;
            processed = true;
        }
        //////////////////////////////////////////////////////////////////////////
        // DELTASK command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "deltask"))
        {
            msg.data = boost::lexical_cast<std::string>(del_task(msg.data));
            retval = 1;
            processed = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // CANCELTASK command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "canceltask"))
        {
            msg.data = boost::lexical_cast<std::string>(cancel_task(msg.data));
            retval = 1;
            processed = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // RUNTASK command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "runtask"))
        {
            msg.data = boost::lexical_cast<std::string>(run_task(msg.data));
            retval = 1;
            processed = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // PAUSETASK command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "pausetask"))
        {
            msg.data = boost::lexical_cast<std::string>(pause_task(msg.data));
            retval = 1;
            processed = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // TRANSPORTS command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "transports"))
        {
            //msg.data = add_task(msg.data);
            retval = 1;
            processed = true;
        }
        //////////////////////////////////////////////////////////////////////////
        // PLUGINS command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "plugins"))
        {
            msg.data = get_plugin_list(msg.data);
            retval = 1;
            processed = true;
        }
        //////////////////////////////////////////////////////////////////////////
        // PLGUI command processing
        //////////////////////////////////////////////////////////////////////////
        if (iequals(msg.cmd, "plgui"))
        {
            msg.data = get_plugin_ui(msg.data);
            retval = 1;
            processed = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // finalization
        //////////////////////////////////////////////////////////////////////////
        if (!processed)
        {
            LOG4CXX_WARN(WeLogger::GetLogger(), "[" << sess->socket().remote_endpoint().address() << "] unknown command: " <<  msg.cmd);
        }
        buff->consume(bufSize);
        {
            ostream oss(buff);
            boost::archive::xml_oarchive oa(oss);
            oa << BOOST_SERIALIZATION_NVP(msg);
        }
        size_t how = buff->size();
//        buff->assign((unsigned char *)(oss.rdbuf()->str()), oss.rdbuf()->pcount());
    }
    catch (std::exception &e)
    {
        LOG4CXX_ERROR(WeLogger::GetLogger(), "[" << sess->socket().remote_endpoint().address() << "] process_message failed: " <<  e.what());
        retval = 0;
    }

    return retval;
}

string get_plugin_list(string filter)
{
    filter = ""; // not used
    string retval = "";
    WePluginList lst = globalDispatcher->PluginList();
    PluginList respList;
    PluginInfo *info;
    WePluginList::iterator plugs;
    
    for(plugs = lst.begin(); plugs != lst.end(); plugs++)
    {
        info = new PluginInfo();
        info->PluginId = (*plugs).PluginId;
        info->PluginDesc = (*plugs).PluginDesc;
        info->IfaceName = (*plugs).IfaceName;
        info->IfaceList.assign((*plugs).IfaceList.begin(), (*plugs).IfaceList.end());
        info->PluginIcon = (*plugs).PluginIcon;
        info->PluginStatus = (*plugs).PluginStatus;
        respList.push_back(*info);
    }
    { // auto-destroy block for output stream
        ostrstream oss;
        { // auto-destroy block for xml_oarchive
            boost::archive::xml_oarchive oa(oss);
            oa << BOOST_SERIALIZATION_NVP(respList);
        }
        retval = string(oss.str(), oss.rdbuf()->pcount());
    }
    return retval;
}

string get_plugin_ui(string filter)
{
    string retval = "";

    iwePlugin* plg = globalDispatcher->LoadPlugin(filter);
    if (plg != NULL) {
        retval = plg->GetSetupUI();
        plg->Release();
    }
    return retval;
}

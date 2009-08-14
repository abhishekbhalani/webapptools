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
#include <weHelper.h>
#include <weTagScanner.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <programcfg.h>

namespace fs = boost::filesystem;

const string ProgramConfig::cfg_version = "0.1";

ProgramConfig::ProgramConfig()
{
    options.clear();
}

ProgramConfig::~ProgramConfig()
{
}

std::string ProgramConfig::plugin_options( const string& plugin_id )
{
    string retval = "";
    map<string, string>::iterator obj;

    obj = options.find(plugin_id);
    if (obj != options.end())
    {
        retval = options[plugin_id];
    }

    return retval;
}

bool ProgramConfig::save_to_file( const string& filename )
{
    bool retval = true;
    string result = "";
    map<string, string>::iterator obj;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "ProgramConfig::save_to_file " << filename);
    result  = "<program_config version='" + cfg_version + "'>\n";
    result += "  <port>" + boost::lexical_cast<string>(port) + "</port>\n";
    result += "  <dbDir>" + WeScreenXML(dbDir) + "</dbDir>\n";
    result += "  <storage>" + WeScreenXML(storageIface) + "</storage>\n";
    for (obj = options.begin(); obj != options.end(); obj++)
    {
        result += "  <plugin_setting plugin_cfg='" + obj->first + "'>";
        result += obj->second;
        result += "</plugin_setting>\n";
    }
    result += "</program_config>\n";

    try {
        ofstream ofs(filename.c_str());
        ofs << result;
        retval = true;
        LOG4CXX_TRACE(WeLogger::GetLogger(), "ProgramConfig::save_to_file - saved");
    }
    catch(const std::exception& e)
    {
        LOG4CXX_ERROR(WeLogger::GetLogger(), "ProgramConfig::save_to_file can't save " << filename << " : " << e.what());
        retval = false;
    }

    return retval;
}

bool ProgramConfig::load_from_file( const string& filename )
{
    bool retval = true;
    string xml = "";
    size_t sz;

    LOG4CXX_TRACE(WeLogger::GetLogger(), "ProgramConfig::load_from_file " << filename);
    try {
        sz = fs::file_size(filename);
        if (sz > 0) {
            char* content = new char[sz+10];
            memset(content, 0, sz+10);
            ifstream ifs(filename.c_str());
            ifs.read(content, sz);
            content[sz] = '\0';
            xml = content;
            delete content;
        }
        LOG4CXX_TRACE(WeLogger::GetLogger(), "ProgramConfig::load_from_file - file loaded");
    }
    catch(const std::exception& e)
    {
        LOG4CXX_ERROR(WeLogger::GetLogger(), "ProgramConfig::load_from_file can't read from " << filename << " : " << e.what());
        retval = false;
    }
    if (retval)
    {
        WeStrStream st(xml.c_str());
        WeTagScanner sc(st);
        int pos_last, pos;
        int parseLevel = 0;
        bool inParsing = true;
        string name, val, dat, plugin_id;
        int token;

        while (inParsing)
        {
            pos = sc.GetPos();
            token = sc.GetToken();
            switch(token)
            {
            case wstError:
                LOG4CXX_WARN(WeLogger::GetLogger(), "ProgramConfig::load_from_file parsing error");
                inParsing = false;
                retval = false;
                break;
            case wstEof:
                LOG4CXX_TRACE(WeLogger::GetLogger(), "ProgramConfig::load_from_file - EOF");
                inParsing = false;
                break;
            case wstTagStart:
                name = sc.GetTagName();
                if (parseLevel == 0)
                {
                    if (iequals(name, "program_config"))
                    {
                        parseLevel = 1;
                        dat = "";
                    }
                    else {
                        LOG4CXX_WARN(WeLogger::GetLogger(), "ProgramConfig::load_from_file unexpected tagStart: " << name);
                        retval = false;
                        inParsing = false;
                    }
                    break;
                }
                if (parseLevel == 1)
                {
                    if (iequals(name, "port"))
                    {
                        parseLevel = 2;
                        dat = "";
                        break;
                    }
                    if (iequals(name, "dbDir"))
                    {
                        parseLevel = 2;
                        dat = "";
                        break;
                    }
                    if (iequals(name, "storage"))
                    {
                        parseLevel = 2;
                        dat = "";
                        break;
                    }
                    if (iequals(name, "plugin_setting"))
                    {
                        pos_last = 0;
                        plugin_id = "";
                        parseLevel = 3;
                        dat = "";
                        break;
                    }
                }
                if (parseLevel == 3)
                {
                    if (pos_last == 0)
                    {
                        pos_last = sc.GetPos() - name.length();
                        // search for opening '<'
                        while (xml[pos_last] != '<') {
                            pos_last--;
                        }
                    }
                    // skip all in plugin_setting
                    break;
                }
                LOG4CXX_WARN(WeLogger::GetLogger(), "ProgramConfig::load_from_file unexpected tagStart: " << name);
                retval = false;
                inParsing = false;
                break;
            case wstTagEnd:
                name = sc.GetTagName();
                if (parseLevel == 1)
                {
                    if (iequals(name, "program_config"))
                    {
                        parseLevel = 0;
                        dat = "";
                        inParsing = false;
                    }
                    else {
                        LOG4CXX_WARN(WeLogger::GetLogger(), "ProgramConfig::load_from_file unexpected wstTagEnd: " << name);
                        retval = false;
                        inParsing = false;
                    }
                    break;
                }
                if (parseLevel == 2)
                {
                    dat = WeUnscreenXML(dat);
                    if (iequals(name, "port"))
                    {
                        port = boost::lexical_cast<short>(dat);
                        parseLevel = 1;
                        break;
                    }
                    if (iequals(name, "dbDir"))
                    {
                        dbDir = dat;
                        parseLevel = 1;
                        break;
                    }
                    if (iequals(name, "storage"))
                    {
                        storageIface = dat;
                        parseLevel = 1;
                        break;
                    }
                }
                if (parseLevel == 3)
                {
                    if (iequals(name, "plugin_setting"))
                    {
                        options[plugin_id] = xml.substr(pos_last, pos - pos_last);
                        parseLevel = 1;
                    }
                    break;
                }
                LOG4CXX_WARN(WeLogger::GetLogger(), "ProgramConfig::load_from_file unexpected wstTagEnd: " << name);
                retval = false;
                inParsing = false;
                break;
            case wstAttr:
                name = sc.GetAttrName();
                val = sc.GetValue();
                val = WeUnscreenXML(val);
                if (parseLevel == 1)
                {
                    if (iequals(name, "version"))
                    {
                        if (cfg_version != val)
                        {
                            LOG4CXX_WARN(WeLogger::GetLogger(), "ProgramConfig::load_from_file read file version " << val <<
                                " to object version " << cfg_version);
                        }
                    }
                }
                if (parseLevel == 3)
                {
                    if (iequals(name, "plugin_cfg") && plugin_id == "")
                    {
                        plugin_id = val;
                    }
                }
                break;
            case wstWord: 
            case wstSpace:
                if (parseLevel == 3 && pos_last == 0)
                {
                        pos_last = sc.GetPos() - strlen(sc.GetValue());
                }
                dat += sc.GetValue();
                break;
            default:
//                 if (parseLevel == 3 && pos_last == 0)
//                 {
//                     pos_last = pos;
//                 }
                break;
            }
        }

    }

    return retval;
}

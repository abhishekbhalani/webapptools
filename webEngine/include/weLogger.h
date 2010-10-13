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
#ifndef __WELOGGING_H__
#define __WELOGGING_H__
#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>

namespace webEngine {

class iLogger {
public:
    static void SetBlock(const std::string& block) {
        m_logger = log4cxx::Logger::getLogger(GetName(block));
    }

    static log4cxx::LoggerPtr GetLogger() {
        return m_logger;
    }
private:
    static std::string GetName(const std::string& block) {
        std::ostringstream name;
        name << "webEngine";
        if (!block.empty())
            name << "." << block;
        return name.str();
    }

public:
    static log4cxx::LoggerPtr	m_logger;
};

} // namespace webEngine

#endif //__WELOGGING_H__

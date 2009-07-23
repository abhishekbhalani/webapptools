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
#ifndef __WEOPTIONS_H__
#define __WEOPTIONS_H__
#include <string>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/variant.hpp>
#include <boost/serialization/variant.hpp>
#include "weiParser.h"

using namespace std;

typedef boost::variant< char,
                        unsigned char,
                        int,
                        unsigned int,
                        long,
                        unsigned long,
                        bool,
                        double,
                        string> WeOptionVal;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  WeOption
///
/// @brief  Options for the WeTask and whole process
///
/// @author A. Abramov
/// @date   09.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class WeOption
{
public:
    WeOption() { empty = true; };
    WeOption(string nm) { name = nm; empty = true; };
    ~WeOption() {};

    //@{
    /// @brief  Access the Name property
    const string &Name(void) const      { return(name); };
    void Name(const string &nm)         { name = nm;    };
    //@}

    //@{
    /// @brief  Access the TypeId property
    // const type_info &Value(void) const     { return(tpId);     };
    template <typename T>
    void GetValue(T& dt)
    { dt = boost::get<T>(val); };
    template <typename T>
    void SetValue(T dt)
    { val = dt; empty = false; };
    //@}

    bool IsEmpty(void)                          { return empty;   };            ///< Is the value empty
    string GetTypeName(void)                    { return val.type().name();};   ///< Gets the value type name
    const std::type_info& GetType(void) const   { return val.type();  };        ///< Gets the value type
    const int Which(void) const                 { return val.which();  };       ///< Gets the internal type

    /// @brief Assignment operator
    WeOption& operator=(WeOption& cpy)
    {   name = cpy.name;
    val = cpy.val;
    empty = cpy.empty;
    return *this; };

#ifndef __DOXYGEN__
protected:
    string      name;
    WeOptionVal val;
    bool        empty;
#endif //__DOXYGEN__

private:
    DECLARE_SERIALIZATOR
    {
        ar & BOOST_SERIALIZATION_NVP(name);
        ar & BOOST_SERIALIZATION_NVP(val);
        empty = false;
    };
};

BOOST_CLASS_TRACKING(WeOption, boost::serialization::track_never)

typedef map<string, WeOption*> WeOptions;

#define SAFE_GET_OPTION_VAL(opt, var, def) try { (opt).GetValue((var));} catch (...) { (var) = (def); };
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  iweOptionsProvider
///
/// @brief  options storage.
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iweOptionsProvider
{
public:
    iweOptionsProvider() {};
    virtual ~iweOptionsProvider() {};

    virtual WeOption& Option(const string& name) = 0;
    virtual bool IsSet(const string& name) = 0;
    virtual void Option(const string& name, WeOptionVal val) = 0;
    virtual void Erase(const string& name)
    {
        WeOptions::iterator it;
        it = options.find(name);
        if (it != options.end()) {
            options.erase(it);
        }
    };

    string ToXml( void );
    void FromXml( string input );
    void FromXml( WeTagScanner& sc, int token = -1 );
#ifndef __DOXYGEN__
protected:
    WeOptions       options;
#endif //__DOXYGEN__

private:
    DECLARE_SERIALIZATOR
    {
        ar & BOOST_SERIALIZATION_NVP(options);
    };
};

//////////////////////////////////////////////////////////////////////////
// Define options names
//////////////////////////////////////////////////////////////////////////
/// object's human readable name or description (string)
#define weoName              ("name")
/// object's identifier (string)
#define weoID                ("id")
/// task status (idle, run, etc) (integer)
#define weoTaskStatus        ("status")
/// task completion (percents) (integer)
#define weoTaskCompletion    ("completion")
#define weoTransport         ("TransportName")
#define weoParser            ("ParserName")
/// put all founded links into the processing queue
#define weoFollowLinks       ("FollowLinks")
/// automatically load images as WeRefrenceObject
#define weoLoadImages        ("LoadImages")
/// automatically load scripts as WeRefrenceObject
#define weoLoadScripts       ("LoadScripts")
/// automatically load frames as WeRefrenceObject
#define weoLoadFrames        ("LoadFrames")
/// collapse multiple spaces into one then HTML parse
#define weoLoadIframes       ("LoadIframes")
/// collapse multiple spaces into one then HTML parse
#define weoCollapseSpaces    ("CollapseSpaces")
/// do not leave domain of the request (second-level or higher)
#define weoStayInDomain      ("StayInDomain")
/// includes weoStayInDomain
#define weoStayInHost        ("StayInHost")
/// includes woeStayInHost & weoStayInDomain
#define weoStayInDir         ("StayInDir")
/// start response processing automatically
#define weoAutoProcess       ("AutoProcess")
/// controls the relocation loops and duplicates
#define weoCheckForLoops     ("CheckForLoops")
//////////////////////////////////////////////////////////////////////////

#endif //__WEOPTIONS_H__

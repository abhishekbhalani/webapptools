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
#include <boost/lexical_cast.hpp>
#include "weiParser.h"

using namespace std;


namespace webEngine {

    class db_recordset;

    typedef ::boost::variant< char,
        unsigned char,
        int,
        unsigned int,
        long,
        unsigned long,
        bool,
        double,
        string> wOptionVal;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  wOption
    ///
    /// @brief  Options for the WeTask and whole process
    ///
    /// @author A. Abramov
    /// @date   09.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class wOption
    {
    public:
        wOption() { empty = true; };
        wOption(string nm) { name = nm; empty = true; };
        ~wOption() {};

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
        {   string tmp;
            tmp = boost::lexical_cast<string>(val);
            dt = boost::lexical_cast<T>(tmp);
        };
        template <typename T>
        void SetValue(T dt)
        { val = dt; empty = false; };
        //@}
        wOptionVal Value() { return val; };

        bool IsEmpty(void)                          { return empty;   };            ///< Is the value empty
        string GetTypeName(void)                    { return val.type().name();};   ///< Gets the value type name
        const ::std::type_info& GetType(void) const { return val.type();  };        ///< Gets the value type
        const int Which(void) const                 { return val.which();  };       ///< Gets the internal type

        /// @brief Assignment operator
        wOption& operator=(wOption& cpy)
        {   name = cpy.name;
        val = cpy.val;
        empty = cpy.empty;
        return *this; };

        bool operator==(wOption& cpy)
        {   name = cpy.name;
        val = cpy.val;
        empty = cpy.empty;
        return (name == cpy.name && val == cpy.val); };

#ifndef __DOXYGEN__
    protected:
        string      name;
        wOptionVal  val;
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

    typedef map<string, wOption*> wOptions;

#define SAFE_GET_OPTION_VAL(opt, var, def) try { (opt).GetValue((var));} catch (...) { (var) = (def); };
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  iOptionsProvider
///
/// @brief  options storage.
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class iOptionsProvider
{
public:
    iOptionsProvider() {};
    virtual ~iOptionsProvider() {};

    virtual wOption& Option(const string& name);
    virtual bool IsSet(const string& name);
    virtual void Option(const string& name, wOptionVal val);
    virtual void Erase(const string& name)
    {
        wOptions::iterator it;
        it = options.find(name);
        if (it != options.end()) {
            options.erase(it);
        }
    };
    virtual void Clear() { options.clear(); };

    void CopyOptions(iOptionsProvider* cpy);
    string_list OptionsList();
    size_t OptionSize() { return options.size(); };

    db_recordset* ToRS( const string& parentID = "" );
    void FromRS( db_recordset *rs );

    // simplified serialization
    string ToXml( void );
    void FromXml( string input );
    void FromXml( TagScanner& sc, int token = -1 );

#ifndef __DOXYGEN__
protected:
    wOptions       options;
#endif //__DOXYGEN__

private:
    DECLARE_SERIALIZATOR
    {
        ar & BOOST_SERIALIZATION_NVP(options);
    };
};

} // namespace webEngine
BOOST_CLASS_TRACKING(webEngine::wOption, boost::serialization::track_never)

//////////////////////////////////////////////////////////////////////////
// Define options names
//////////////////////////////////////////////////////////////////////////
/// object's human readable name or description (string)
#define weoName              "name"
/// object's identifier (string)
#define weoID                "id"
/// object's type
#define weoTypeID            "type"
/// object's value
#define weoValue             "value"
/// task status (idle, run, etc) (integer)
#define weoTaskStatus        "status"
/// task completion (percents) (integer)
#define weoTaskCompletion    "completion"
#define weoTransport         "TransportName"
#define weoParser            "ParserName"
/// put all founded links into the processing queue (bool)
#define weoFollowLinks       "FollowLinks"
/// automatically load images as WeRefrenceObject (bool)
#define weoLoadImages        "LoadImages"
/// automatically load scripts as WeRefrenceObject (bool)
#define weoLoadScripts       "LoadScripts"
/// automatically load frames as WeRefrenceObject (bool)
#define weoLoadFrames        "LoadFrames"
/// automatically load iframes as WeRefrenceObject (bool)
#define weoLoadIframes       "LoadIframes"
/// collapse multiple spaces into one then HTML parse (bool)
#define weoCollapseSpaces    "CollapseSpaces"
/// do not leave domain of the request (second-level or higher) (bool)
#define weoStayInDomain      "StayInDomain"
/// includes weoStayInDomain (bool)
#define weoStayInHost        "StayInHost"
/// includes woeStayInHost & weoStayInDomain (bool)
#define weoStayInDir         "StayInDir"
/// start response processing automatically (bool)
#define weoAutoProcess       "AutoProcess"
/// controls the relocation loops and duplicates (bool)
#define weoCheckForLoops     "CheckForLoops"
/// base URL for processing (bool)
#define weoBaseURL           "BaseURL"
/// links following depth (integer)
#define weoScanDepth         "ScanDepth"
/// logging level (integer)
#define weoLogLevel          "LogLevel"
/// number of parallel requests to transport (integer)
#define weoParallelReq       "ParallelReq"
/// ignore URL parameters (bool)
#define weoIgnoreUrlParam    "noParamUrl"
/// identifiers of the parent object (string)
#define weoParentID          "ParentId"
/// identifiers of the profile object (string)
#define weoProfileID         "ProfileId"
/// signal to the task (int)
#define weoTaskSignal        "signal"
//////////////////////////////////////////////////////////////////////////
// Define options typenames
//////////////////////////////////////////////////////////////////////////
#define weoTypeInt           "2"
#define weoTypeUInt          "3"
#define weoTypeBool          "6"
#define weoTypeString        "8"
//////////////////////////////////////////////////////////////////////////
// Define task statuses
//////////////////////////////////////////////////////////////////////////
#define WI_TSK_IDLE     0
#define WI_TSK_RUN      1
#define WI_TSK_PAUSED   2
#define WI_TSK_MAX      3
//////////////////////////////////////////////////////////////////////////

#endif //__WEOPTIONS_H__

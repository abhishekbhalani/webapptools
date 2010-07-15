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
#include <boost/blank.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/lexical_cast.hpp>
#include "weTagScanner.h"
#include "weStrings.h"
#include "weVariant.h"

using namespace std;


namespace webEngine {

    class db_recordset;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @class  we_option
    ///
    /// @brief  Options for the WeTask and whole process
    ///
    /// @author A. Abramov
    /// @date   09.06.2009
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    class we_option
    {
    public:
        we_option() { val = boost::blank(); }
        we_option(const we_option& c) { oname = c.oname; val = c.val; }
        we_option(string n) { oname = n; val = boost::blank(); }
        we_option(string n, we_variant v) { oname = n; val = v; }
        ~we_option() {};

        //@{
        /// @brief  Access the name property
        const string &name(void) const      { return(oname); };
        void name(const string &nm)         { oname = nm;    };
        //@}

        //@{
        /// @brief  Access the TypeId property
        // const type_info &Value(void) const     { return(tpId);     };
        template <typename T>
        void GetValue(T& dt)
        { dt = val.get<T>(); }

        template <typename T>
        void SetValue(T dt)
        { val = dt; }
        //@}
        we_variant& Value() { return val; }

        bool IsEmpty(void)                          { return val.empty(); }         ///< Is the value empty
        string GetTypeName(void)                    { return val.type().name(); }   ///< Gets the value type name
        const std::type_info& GetType(void) const   { return val.type();  }         ///< Gets the value type

        /// @brief Assignment operator
        we_option& operator=(we_option& cpy)
        {   oname = cpy.oname;
            val = cpy.val;
            return *this;
        }

        bool operator==(we_option& cpy)
        {
            bool result = (oname == cpy.oname);
            result = result && (val == cpy.val);
            return result;
        }

#ifndef __DOXYGEN__
    protected:
        string      oname;
        we_variant  val;
#endif //__DOXYGEN__
    };

    typedef map<string, we_option> wOptions;

#define SAFE_GET_OPTION_VAL(opt, var, def) try { (opt).GetValue((var));} catch (boost::bad_lexical_cast &) { (var) = (def); };
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @interface  i_options_provider
///
/// @brief  options storage.
///
/// @author A. Abramov
/// @date   10.06.2009
////////////////////////////////////////////////////////////////////////////////////////////////////
class i_options_provider
{
public:
    i_options_provider() {};
    virtual ~i_options_provider() {};

    virtual we_option Option(const string& name) = 0;
    virtual bool IsSet(const string& name) = 0;
    virtual void Option(const string& name, we_variant val) = 0;
    virtual void Erase(const string& name) = 0;
    virtual void Clear() = 0;

    virtual void CopyOptions(i_options_provider* cpy);
    virtual string_list OptionsList() = 0;
    virtual size_t OptionSize() = 0;

    static we_option empty_option;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class  options_provider
///
/// @brief  In-memory options storage.
///
/// @author A. Abramov
/// @date   29.04.2010
////////////////////////////////////////////////////////////////////////////////////////////////////
class options_provider : public i_options_provider
{
public:
    options_provider() {};
    virtual ~options_provider();

    virtual we_option Option(const string& name);
    virtual bool IsSet(const string& name);
    virtual void Option(const string& name, we_variant val);
    virtual void Erase(const string& name)
    {
        wOptions::iterator it;
        it = options.find(name);
        if (it != options.end()) {
            options.erase(it);
        }
    };
    virtual void Clear() { options.clear(); };
    virtual string_list OptionsList();
    virtual size_t OptionSize() { return options.size(); };

    db_recordset* ToRS( const string& parentID = "" );
    void FromRS( db_recordset *rs );

    // simplified serialization
    string ToXml( void );
    void FromXml( string input );
    void FromXml( tag_scanner& sc, int token = -1 );

#ifndef __DOXYGEN__
protected:
    wOptions       options;
#endif //__DOXYGEN__
};

} // namespace webEngine
BOOST_CLASS_TRACKING(webEngine::we_option, boost::serialization::track_never)

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
#define weoTransport         "transport_name"
#define weoParser            "parser_name"
/// put all founded links into the processing queue (bool)
#define weoFollowLinks       "follow_links"
/// automatically load images as WeRefrenceObject (bool)
#define weoLoadImages        "load_images"
/// automatically load scripts as WeRefrenceObject (bool)
#define weoLoadScripts       "load_scripts"
/// automatically load frames as WeRefrenceObject (bool)
#define weoLoadFrames        "load_frames"
/// automatically load iframes as WeRefrenceObject (bool)
#define weoLoadIframes       "load_iframes"
/// collapse multiple spaces into one then HTML parse (bool)
#define weoCollapseSpaces    "collapse_spaces"
/// do not leave domain of the request (second-level or higher) (bool)
#define weoStayInDomain      "stay_in_domain"
/// includes weoStayInDomain (bool)
#define weoStayInHost        "stay_in_host"
/// includes woeStayInHost & weoStayInDomain (bool)
#define weoStayInDir         "stay_in_dir"
/// start response processing automatically (bool)
#define weoAutoProcess       "auto_process"
/// controls the relocation loops and duplicates (bool)
#define weoCheckForLoops     "check_for_loops"
/// base URL for processing (bool)
#define weoBaseURL           "base_url"
/// links following depth (integer)
#define weoScanDepth         "scan_depth"
/// logging level (integer)
#define weoLogLevel          "log_level"
/// number of parallel requests to transport (integer)
#define weoParallelReq       "parallel_req"
/// semicolon separated list of the denied file types (by extensions)
#define weoDeniedFileTypes   "denied_file_types"
/// semicolon separated list of the allowed sub-domains
#define weoDomainsAllow      "domains_allow"
/// ignore URL parameters (bool)
#define weoIgnoreUrlParam    "no_param_url"
/// identifiers of the parent object (string)
#define weoParentID          "parent_id"
/// identifiers of the profile object (string)
#define weoProfileID         "profile_id"
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

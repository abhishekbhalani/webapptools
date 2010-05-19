#include "weJsExecutor.h"
#include <weiAudit.h>
#include <weUrl.h>
#include <weScan.h>
#include <weiParser.h>

class audit_jscript :
    public webEngine::i_audit
{
public:
    audit_jscript(webEngine::engine_dispatcher* krnl, void* handle = NULL);
    ~audit_jscript(void);

    // i_plugin functions
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual void* get_interface(const string& ifName)
    ///
    /// @brief  Gets an interface.
    ///
    /// Returns abstracted pointer to the requested interface, or NULL if the requested interface isn't
    /// provided by this object. The interface name depends on the plugin implementation.
    ///
    /// @param  ifName - Name of the interface.
    ///
    /// @retval	null if it fails, else the interface.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual i_plugin* get_interface(const string& ifName);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn virtual const string get_setup_ui( void )
    ///
    /// @brief  Gets the user interface for the setup dialog. 
    ///
    /// @retval The user interface in the XML-based format or empty string if no setup dialog. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual const string get_setup_ui( void ) { return ""; };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void init(task* tsk)
    ///
    /// @brief  Initialize plugin for given task. 
    ///
    /// @param  tsk	   - If non-null, the pointer to task what handles the process. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void init(webEngine::task* tsk);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void start(task* tsk, ScanData* scData)
    ///
    /// @brief  Starts the audit process for given ScanData object. 
    ///
    /// @param  tsk	   - If non-null, the pointer to task what handles the process. 
    /// @param  scData - If non-null, the pointer to scan data what contains values to audit. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void start(webEngine::task* tsk, boost::shared_ptr<webEngine::ScanData> scData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void process_response(i_response *resp)
    ///
    /// @brief  Process the transport response described by resp.
    /// 		
    /// @param  resp - If non-null, the resp. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void process_response(webEngine::i_response_ptr resp);

    virtual void parse_scripts(boost::shared_ptr<webEngine::ScanData> sc, boost::shared_ptr<webEngine::HtmlDocument> parser);
    virtual void process_events(webEngine::iEntityPtr entity);

    static webEngine::jsExecutor* js_exec;

protected:
    void add_url( webEngine::transport_url link, boost::shared_ptr<webEngine::ScanData> sc );
    void extract_links(string text, boost::shared_ptr<webEngine::ScanData> sc );
    friend void parser_thread(void* context);

    webEngine::transport_url base_path;
    // real data type is webEngine::iEntity
    vector<void*> to_download;
    map<string, int> scandatas;
    vector<string> ext_deny;

    // processing options
    bool opt_in_host;
    bool opt_in_domain;
    bool opt_ignore_param;
    int  opt_max_depth;
};

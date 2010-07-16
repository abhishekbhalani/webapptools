#include <weiAudit.h>
#include <weUrl.h>
#include <weScan.h>

class audit_comment :
    public webEngine::i_audit
{
public:
    audit_comment(webEngine::engine_dispatcher* krnl, void* handle = NULL);
    ~audit_comment(void);

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
    /// @fn void process(task* tsk, ScanData* scData)
    ///
    /// @brief  Starts the audit process for given ScanData object. 
    ///
    /// @param  tsk	   - If non-null, the pointer to task what handles the process. 
    /// @param  scData - If non-null, the pointer to scan data what contains values to audit. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void process(webEngine::task* tsk, boost::shared_ptr<webEngine::ScanData> scData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void process_response(i_response *resp)
    ///
    /// @brief  Process the transport response described by resp.
    /// 		
    /// @param  resp - If non-null, the resp. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void process_response(webEngine::i_response_ptr resp);

protected:
    void add_url( webEngine::transport_url link, webEngine::transport_url *base_url, boost::shared_ptr<webEngine::ScanData> scData );
    void extract_links(string text, webEngine::transport_url *base_url, boost::shared_ptr<webEngine::ScanData> scData );

    vector<string> ext_deny;
    int ctype_method;

    // processing options
    bool opt_in_host;
    bool opt_in_domain;
    int  opt_ignore_param;
    int  opt_max_depth;
};

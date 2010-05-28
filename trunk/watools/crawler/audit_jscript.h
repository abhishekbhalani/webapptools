#include "weJsExecutor.h"
#include <weiAudit.h>
#include <weUrl.h>
#include <weScan.h>
#include <weiParser.h>
#include <weHtmlEntity.h>

class audit_jscript;

class ajs_to_process :
    public boost::noncopyable
{
public:
    explicit ajs_to_process(webEngine::scan_data_ptr sc = webEngine::scan_data_ptr(),
        webEngine::html_document_ptr pd = webEngine::html_document_ptr());

    webEngine::scan_data_ptr sc_data;
    webEngine::html_document_ptr parsed_data;
    int pending_requests;
};

typedef boost::shared_ptr<ajs_to_process> ajs_to_process_ptr;
typedef vector<ajs_to_process_ptr> ajs_to_process_list;
typedef pair<webEngine::base_entity_ptr, ajs_to_process_ptr> ajs_download;
typedef vector<ajs_download>  ajs_download_list;
typedef pair<string, ajs_download_list>  ajs_download_task;

class ajs_download_queue :
    public boost::noncopyable
{
// the URL is the key!
public:
    ajs_download_queue() { task_list.clear(); process_list.clear(); }

    webEngine::i_request_ptr add(string url, ajs_to_process_ptr tp, webEngine::base_entity_ptr ent);
    bool add(ajs_to_process_ptr tp);
    void remove_request(string req_id);
    void clean_done();

    map<string, ajs_download_task> task_list;
    typedef map<string, ajs_download_task>::iterator iterator;
    ajs_to_process_list process_list;
};
#define AJS_DOWNLOAD_URL(it)    (it)->first
#define AJS_DOWNLOAD_ID(it)     (it)->second.first
#define AJS_DOWNLOAD_LIST(it)   (it)->second.second

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
    virtual void start(webEngine::task* tsk, webEngine::scan_data_ptr scData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @fn void process_response(i_response *resp)
    ///
    /// @brief  Process the transport response described by resp.
    /// 		
    /// @param  resp - If non-null, the resp. 
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void process_response(webEngine::i_response_ptr resp);

    virtual void parse_scripts(webEngine::scan_data_ptr sc, boost::shared_ptr<webEngine::html_document> parser);
    virtual void process_events(webEngine::base_entity_ptr entity);

    static webEngine::jsExecutor* js_exec;

protected:
    void add_url( webEngine::transport_url link, webEngine::scan_data_ptr sc );
    void extract_links(string text, webEngine::scan_data_ptr sc );
    friend void parser_thread(audit_jscript* object);

    webEngine::transport_url base_path;
    vector<string> ext_deny;
    ajs_download_queue jscript_tasks;
    boost::mutex data_access;
    boost::mutex thread_synch;
    boost::condition_variable thread_event;
    bool thread_running;

    // processing options
    bool opt_in_host;
    bool opt_in_domain;
    bool opt_ignore_param;
    int  opt_max_depth;
};

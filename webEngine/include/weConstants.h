#ifndef _WE_CONSTANTS__2011_04_06__
#define _WE_CONSTANTS__2011_04_06__

#define CURLAUTH_FORMS      (1<<15)

/// allowed all hosts on same IP address
#define weoStayInIP          "httpInventory/stay_in_ip"
/// allowed all domains in the list
#define weoStayInDomainList  "httpInventory/stay_in_domain_list"
/// do not leave domain of the request (second-level or higher) (bool)
#define weoStayInDomain      "httpInventory/stay_in_domain"
/// includes weoStayInDomain (bool)
#define weoStayInHost        "httpInventory/stay_in_host"
/// includes woeStayInHost & weoStayInDomain (bool)
#define weoStayInDir         "httpInventory/stay_in_dir"
/// links following depth (integer)
#define weoScanDepth         "httpInventory/scan_depth"
/// semicolon separated list of the denied file types (by extensions)
#define weoDeniedFileTypes   "httpInventory/denied_file_types"
/// semicolon separated list of the allowed sub-domains
#define weoDomainsAllow      "httpInventory/domains_allow"
/// URL parameters processing mode
#define weoIgnoreUrlParam    "httpInventory/url_param"
/// allowed content-type
#define weoAllowedCTypes     "httpInventory/AllowedCType"
/// list of urls to ignore
#define weoIgnoreUrlList     "httpInventory/ignore_urls"
/// use HTTP-Basic authentication
#define weoHttpAuthBasic     "httpInventory/Auth/Base"
/// use HTTP-Basic authentication
#define weoHttpAuthDigest    "httpInventory/Auth/Digest"
/// use HTTP-Basic authentication
#define weoHttpAuthNTLM      "httpInventory/Auth/NTLM"
/// use HTTP-Basic authentication
#define weoHttpAuthForms     "httpInventory/Auth/Forms"
/// domain for NTLM authentication
#define weoHttpAuthDomain    "httpInventory/Auth/domain"
/// username for authentication
#define weoHttpAuthUname     "httpInventory/Auth/username"
/// password for authentication
#define weoHttpAuthPassword  "httpInventory/Auth/password"
/// form descriptions for authentication
#define weoHttpAuthFormData  "httpInventory/Auth/form-list"


/// base URL for processing
#define weoBaseURL           "base_url"
/// number of parallel requests to transport (integer)
#define weoParallelReq       "parallel_req"
/// hostname for scanning
#define weoScanHost          "scan_host"


#endif //_WE_CONSTANTS__2011_04_06__
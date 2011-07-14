#ifndef DNS_RESOLVER__H__
#define DNS_RESOLVER__H__

#include <vector>
#include <string>

namespace dns_resolver {
/**
 * Get ips and alias from dns server
 * @param [in] const std::string & hostname - hostname (ex. "www.google.com")
 * @param [out] std::vector<std::string> & ip - list of ips
 * @param [out] std::vector<std::string> & alias - list of alias
 * @return int - error code. zero is success
 */
int resolve(const std::string &hostname, std::vector<std::string> &ip, std::vector<std::string> &alias);
}

#endif
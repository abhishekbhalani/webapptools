<<<<<<< .mine

#include "dns_resolver.h"

using namespace std;

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Windns.h>
#include <Winsock2.h>

#pragma comment(lib, "Dnsapi.lib")
#pragma comment(lib, "Ws2_32.lib")

int dns_resolver::resolve(const std::string &hostname, std::vector<std::string> &ip, std::vector<std::string> &alias)
{
    PDNS_RECORD pDns = NULL;
    char *str = NULL;
    DWORD dwRet = 0;

    if( ( dwRet = DnsQuery_A( hostname.c_str(), 1, DNS_QUERY_STANDARD, NULL, &pDns, NULL ) ) != 0 ) return dwRet;

    for (PDNS_RECORD currentDns = pDns; currentDns != NULL; currentDns = currentDns->pNext) {
        switch(currentDns->wType) {
        case DNS_TYPE_A:
            str = inet_ntoa( *((in_addr*)&(currentDns->Data.A.IpAddress)));
            if(str)
                ip.push_back(str);
            break;
        case DNS_TYPE_CNAME:
            if(currentDns->Data.CNAME.pNameHost)
                alias.push_back(currentDns->Data.CNAME.pNameHost);
            break;
        }
    }

    if( pDns ) DnsRecordListFree( pDns, DnsFreeRecordList );

    return dwRet;
}

#else

int dns_resolver::resolve(const std::string &hostname, std::vector<std::string> &ip, std::vector<std::string> &alias)
{
    return -1;
}

#endif
=======

#include "dns_resolver.h"

using namespace std;

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Windns.h>
#include <Winsock2.h>

#pragma comment(lib, "Dnsapi.lib")
#pragma comment(lib, "Ws2_32.lib")

int dns_resolver::resolve(const std::string &hostname, std::vector<std::string> &ip, std::vector<std::string> &alias)
{
    PDNS_RECORD pDns = NULL;
    char *str = NULL;
    DWORD dwRet = 0;

    if( ( dwRet = DnsQuery_A( hostname.c_str(), 1, DNS_QUERY_STANDARD, NULL, &pDns, NULL ) ) != 0 ) return dwRet;

    for (PDNS_RECORD currentDns = pDns; currentDns != NULL; currentDns = currentDns->pNext) {
        switch(currentDns->wType) {
        case DNS_TYPE_A:
            str = inet_ntoa( *((in_addr*)&(currentDns->Data.A.IpAddress)));
            if(str)
                ip.push_back(str);
            break;
        case DNS_TYPE_CNAME:
            if(currentDns->Data.CNAME.pNameHost)
                alias.push_back(currentDns->Data.CNAME.pNameHost);
            break;
        }
    }

    if( pDns ) DnsRecordListFree( pDns, DnsFreeRecordList );

    return dwRet;
}

#else

int dns_resolver::resolve(const std::string &hostname, std::vector<std::string> &ip, std::vector<std::string> &alias)
{
    return -1;
}

#endif
>>>>>>> .r405

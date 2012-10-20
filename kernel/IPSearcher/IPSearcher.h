/*
 * IPSearcher 3.2 (x86)
 * 版权所有 ZX Inc.
 */

#ifndef IPS_H_
#define IPS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DB_MYIPV4  1
#define DB_QQWRY   2
#define DB_IPV6WRY 4
#define DB_MYIPV6  8
#define DB_MYIP    (DB_MYIPV4|DB_MYIPV6)
#define DB_ALL     (DB_MYIP|DB_QQWRY|DB_IPV6WRY)

#ifndef TAGADDR
#define TAGADDR
typedef struct Address{
	char *Country, *Local;
}Addr, *pAddr;
typedef struct AddressW{
	wchar_t *Country, *Local;
}AddrW, *pAddrW;
#endif

pAddr  __cdecl   _GetAddress    (const char   * ip);
pAddr  __stdcall GetAddressA    (const char   * ip);
pAddrW __stdcall GetAddressW    (const wchar_t* ip);
unsigned __int64 __stdcall GetRecordCount(const unsigned int idb);
bool   __stdcall IsIpv6A        (const char   * ip);
bool   __stdcall IsIpv6W        (const wchar_t* ip);
void   __cdecl   LookupAddress  (const char   * ip, pAddr  address);
void   __stdcall LookupAddressA (const char   * ip, pAddr  address);
void   __stdcall LookupAddressW (const wchar_t* ip, pAddrW address);
int    __stdcall ReloadDatabase ();
int    __stdcall ReloadDatabaseEx (const unsigned int idb);
int    __stdcall ReloadDatabaseWithPathA (const unsigned int idb, const char * fn);
int    __stdcall ReloadDatabaseWithPathW (const unsigned int idb, const wchar_t * fn);
size_t __stdcall lstrcntA       (const char   * str, const char   * strtocount);
size_t __stdcall lstrcntW       (const wchar_t* str, const wchar_t* strtocount);

/*
 * pAddr  也可定义为 char   ** 类型
 * pAddrW 也可定义为 wchar_t** 类型
 */

#ifdef __cplusplus
}
#endif

#endif

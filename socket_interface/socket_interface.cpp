#include <cstringt.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <assert.h>
#include <stdio.h>
#include <process.h>
#include <string>

#pragma comment(lib,"Ws2_32.lib")

#include "socket_interface.h"

SOCKET InitWSAData(WSADATA &WSAData)
{
	if   (WSAStartup   (MAKEWORD(2,   2)   ,   &WSAData)   !=   0)     
	{
		return SOCKET_ERROR;   
	}   
	if   (LOBYTE(WSAData.wVersion)   !=   2   ||   HIBYTE(WSAData.wVersion)   !=   2   )   {   
		WSACleanup();  
		return SOCKET_ERROR;
	}

	return 0;
}


bool get_local_ip(char ***ipv4, char ***ipv6, int *ipv4_num, int *ipv6_num)
{
	*ipv4 = NULL;
	*ipv6 = NULL;
	WSADATA   WSAData;   
	if(SOCKET_ERROR == InitWSAData(WSAData))
		return false;

	int iResult; 
    int i = 1;  
    struct addrinfo *result = NULL;  
    struct addrinfo *ptr = NULL;  
    struct addrinfo hints; 
    LPSOCKADDR sockaddr_ip;  
    char ipstringbuffer[51];
    DWORD ipbufferlength;
	memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    char host_name[256] = {0};
    iResult = gethostname(host_name, 256);  
	if (iResult != 0)
        return false;
    iResult = getaddrinfo(host_name, NULL, &hints, &result);
    if (iResult != 0)
        return false;

	int ipv4_total_len=0, ipv6_total_len=0;
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
    {
        sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;  
        ipbufferlength = 51;
        iResult = WSAAddressToStringA(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL,   
               ipstringbuffer, &ipbufferlength ); 
		if(ptr->ai_addr->sa_family == AF_INET6)
		{
            if (iResult==0 && strncmp(ipstringbuffer,"fe80",4)!=0 && strcmp(ipstringbuffer,"::1") !=0)
		    {
			     ipv6_total_len += (strlen(ipstringbuffer) + 1);
				 ++(*ipv6_num);
		    }
		}
		else if(ptr->ai_addr->sa_family == AF_INET)
		{
			ipv4_total_len += (strlen(ipstringbuffer) + 1);
			++(*ipv4_num);
		}
    }
	char **ipv4_array = NULL, **ipv6_array = NULL, *tmipv4_array = NULL, *tmipv6_array = NULL;
	if((*ipv4_num) > 0)
	{
	    ipv4_array = (char **)malloc(sizeof(char *)*(*ipv4_num));
	    tmipv4_array = (char *)malloc(ipv4_total_len*(*ipv4_num));
	}
	if((*ipv6_num) > 0)
	{
	    ipv6_array = (char **)malloc(sizeof(char *)*(*ipv6_num));
	    tmipv6_array = (char *)malloc(ipv6_total_len*(*ipv6_num));
	}

	int i_v4 = 0, i_v6 = 0;
	ipv4_total_len=0;
	ipv6_total_len=0;
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
    {
        sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;  
        ipbufferlength = 51;
        iResult = WSAAddressToStringA(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL,   
               ipstringbuffer, &ipbufferlength ); 
		if(ptr->ai_addr->sa_family == AF_INET6)
		{
            if (iResult==0 && strncmp(ipstringbuffer,"fe80",4)!=0 && strcmp(ipstringbuffer,"::1") !=0)
		    {
			    ipv6_array[i_v6] = tmipv6_array + ipv6_total_len;
				int ipv6_len = (strlen(ipstringbuffer) + 1);
				strcpy_s(ipv6_array[i_v6], ipv6_len, ipstringbuffer);
				ipv6_total_len += ipv6_len;
				i_v6++;
		    }
		}
		else if(ptr->ai_addr->sa_family == AF_INET)
		{
			ipv4_array[i_v4] = tmipv4_array + ipv4_total_len;
			int ipv4_len = (strlen(ipstringbuffer) + 1);
			strcpy_s(ipv4_array[i_v4], ipv4_len, ipstringbuffer);
			ipv4_total_len += ipv4_len;
			i_v4++;
		}
    }

	*ipv4 = ipv4_array;
	*ipv6 = ipv6_array;
    freeaddrinfo(result);
	WSACleanup();
	return true;
}

bool multicast_string(const char *ip, const char *port, int ttl, const char *string)
{
	int stringlen = strlen(string) + 1;
	if(stringlen > BUFFER_SIZE)
		return false;

	SOCKET sockfd;
	WSADATA   WSAData;   
	if(SOCKET_ERROR == InitWSAData(WSAData))
		return false;

	ADDRINFOA rinfot,*multicastAddr;
	memset(&rinfot,0,sizeof(rinfot));
	rinfot.ai_family=AF_UNSPEC;
	rinfot.ai_socktype=SOCK_DGRAM;
	rinfot.ai_flags=AI_NUMERICHOST;                    //地址为字符串
	int iResult = getaddrinfo(ip,port,&rinfot,&multicastAddr);
	if (iResult != 0)
        return false;

	if   ((sockfd=socket(multicastAddr->ai_family,multicastAddr->ai_socktype,0))== INVALID_SOCKET)   {  
		freeaddrinfo(multicastAddr);
		WSACleanup();
		return false;
	}

	//设置存活时间
	if(setsockopt(sockfd,
		multicastAddr->ai_family == PF_INET6 ? IPPROTO_IPV6        : IPPROTO_IP,
		multicastAddr->ai_family == PF_INET6 ? IPV6_MULTICAST_HOPS : IP_MULTICAST_TTL,
		(char   *)&ttl,
		sizeof(ttl))!=0
		)     
	{
		closesocket(sockfd);
		freeaddrinfo(multicastAddr);
		WSACleanup();
		return false;
	}

	int sendedlen = 0;
	if((sendedlen = sendto(sockfd,  string, stringlen, 0,  multicastAddr->ai_addr, multicastAddr->ai_addrlen))   
			==   SOCKET_ERROR)
	{
		closesocket(sockfd);
		freeaddrinfo(multicastAddr);
		WSACleanup();
		return false;
	}

	closesocket(sockfd);
	freeaddrinfo(multicastAddr);
	WSACleanup();

	if(sendedlen == stringlen)
	    return true;
	else
		return false;
}

bool send_udp_string(const char *ip, const char *port, const char *string)
{
	int stringlen = strlen(string) + 1;
	if(stringlen > BUFFER_SIZE)
		return false;

	WSADATA   WSAData;   
	if(SOCKET_ERROR == InitWSAData(WSAData))
		return false;

	ADDRINFOA rinfot,*peerAddr;
	memset(&rinfot,0,sizeof(rinfot));
	rinfot.ai_family=AF_UNSPEC;
	rinfot.ai_socktype=SOCK_DGRAM;
	rinfot.ai_protocol=IPPROTO_UDP;
	rinfot.ai_flags=AI_NUMERICHOST;
	int iResult = getaddrinfo(ip,port,&rinfot,&peerAddr);
	if (iResult != 0)
        return false;

	SOCKET sockfd;
	if   ((sockfd=socket(peerAddr->ai_family,peerAddr->ai_socktype,0))== INVALID_SOCKET)   {  
		freeaddrinfo(peerAddr);
		WSACleanup();
		return false;
	}

	int sendedlen = 0;
	if((sendedlen = sendto(sockfd,  string, stringlen, 0,  peerAddr->ai_addr, peerAddr->ai_addrlen))   
			==   SOCKET_ERROR)
	{
		closesocket(sockfd);
		freeaddrinfo(peerAddr);
		WSACleanup();
		return false;
	}

	closesocket(sockfd);
	freeaddrinfo(peerAddr);
	WSACleanup();

	if(sendedlen == stringlen)
	    return true;
	else
		return false;
}

bool send_udp_string_by_socket(const SOCKET sockfd, const char *ip, const char *port, const char *string)
{
	if(SOCKET_ERROR == sockfd)
		return false;

	int stringlen = strlen(string) + 1;
	if(stringlen > BUFFER_SIZE)
		return false;

	ADDRINFOA rinfot,*peerAddr;
	memset(&rinfot,0,sizeof(rinfot));
	rinfot.ai_family=AF_UNSPEC;
	rinfot.ai_socktype=SOCK_DGRAM;
	rinfot.ai_protocol=IPPROTO_UDP;
	rinfot.ai_flags=AI_NUMERICHOST;
	int iResult = getaddrinfo(ip,port,&rinfot,&peerAddr);
	if (iResult != 0)
        return false;

	int sendedlen = 0;
	if((sendedlen = sendto(sockfd,  string, stringlen, 0,  peerAddr->ai_addr, peerAddr->ai_addrlen))   
			==   SOCKET_ERROR)
	{
		freeaddrinfo(peerAddr);
		return false;
	}

	if(sendedlen == stringlen)
	    return true;
	else
		return false;
}

bool send_tcp_string(const char *ip, const char *port, const char *string)
{
	WSADATA   WSAData;   
	if(SOCKET_ERROR == InitWSAData(WSAData))
		return false;

	ADDRINFOA rinfot,*peerAddr;
	memset(&rinfot,0,sizeof(rinfot));
	rinfot.ai_family=AF_UNSPEC;
	rinfot.ai_socktype=SOCK_STREAM;
	rinfot.ai_protocol=IPPROTO_TCP;
	int iResult = getaddrinfo(ip,port,&rinfot,&peerAddr);
	if (iResult != 0)
        return false;

	SOCKET sockfd;
	if   ((sockfd=socket(peerAddr->ai_family,peerAddr->ai_socktype,0))== INVALID_SOCKET)   {  
		freeaddrinfo(peerAddr);
		WSACleanup();
		return false;
	}

	if(connect(sockfd, peerAddr->ai_addr, peerAddr->ai_addrlen) == SOCKET_ERROR){
		closesocket(sockfd);
		freeaddrinfo(peerAddr);
		WSACleanup();
		return false;
	}
	freeaddrinfo(peerAddr);

	size_t stringlen = strlen(string)+1;
	int ret = send(sockfd, (const char *)&stringlen, sizeof(size_t), 0);
	if(SOCKET_ERROR == ret)
	{
		closesocket(sockfd);
		WSACleanup();
		return false;
	}

	size_t sendlen = 0, sendedlen = 0;
	while(sendedlen < stringlen)
	{
		sendlen = send(sockfd, string+sendedlen, stringlen-sendedlen, 0);
		if(SOCKET_ERROR == sendlen)
		{
			closesocket(sockfd);
		    WSACleanup();
		    return false;
		}

		sendedlen += sendlen;
	}

	closesocket(sockfd);
	WSACleanup();
	return true;
}

bool send_tcp_string_without_closesocket(const char *ip, const char *port, const char *string, SOCKET *sock)
{
	WSADATA   WSAData;   
	if(SOCKET_ERROR == InitWSAData(WSAData))
		return false;

	ADDRINFOA rinfot,*peerAddr;
	memset(&rinfot,0,sizeof(rinfot));
	rinfot.ai_family=AF_UNSPEC;
	rinfot.ai_socktype=SOCK_STREAM;
	rinfot.ai_protocol=IPPROTO_TCP;
	int iResult = getaddrinfo(ip,port,&rinfot,&peerAddr);
	if (iResult != 0)
		return false;

	SOCKET sockfd;
	if   ((sockfd=socket(peerAddr->ai_family,peerAddr->ai_socktype,0))== INVALID_SOCKET)   {  
		freeaddrinfo(peerAddr);
		WSACleanup();
		return false;
	}

	if(connect(sockfd, peerAddr->ai_addr, peerAddr->ai_addrlen) == SOCKET_ERROR){
		closesocket(sockfd);
		freeaddrinfo(peerAddr);
		WSACleanup();
		return false;
	}
	freeaddrinfo(peerAddr);

	size_t stringlen = strlen(string)+1;
	int ret = send(sockfd, (const char *)&stringlen, sizeof(size_t), 0);
	if(SOCKET_ERROR == ret)
	{
		closesocket(sockfd);
		WSACleanup();
		return false;
	}

	size_t sendlen = 0, sendedlen = 0;
	while(sendedlen < stringlen)
	{
		sendlen = send(sockfd, string+sendedlen, stringlen-sendedlen, 0);
		if(SOCKET_ERROR == sendlen)
		{
			closesocket(sockfd);
			WSACleanup();
			return false;
		}

		sendedlen += sendlen;
	}

	*sock = sockfd;
	return true;
}

typedef struct Async_Send_Package
{
	char *ip;
	char *port;
	char *string;
	ASYNCSENDCALLBACKPROC lpCallBack;
}async_send_package;

unsigned int __stdcall async_send_tcp_string_thread(LPVOID pParam)
{
	async_send_package *pkg = (async_send_package *)pParam;

	WSADATA   WSAData;   
	if(SOCKET_ERROR == InitWSAData(WSAData))
		return false;

	ADDRINFOA rinfot,*peerAddr;
	memset(&rinfot,0,sizeof(rinfot));
	rinfot.ai_family=AF_UNSPEC;
	rinfot.ai_socktype=SOCK_STREAM;
	rinfot.ai_protocol=IPPROTO_TCP;
	int iResult = getaddrinfo(pkg->ip,pkg->port,&rinfot,&peerAddr);
	if (iResult != 0)
        return false;

	SOCKET sockfd;
	if   ((sockfd=socket(peerAddr->ai_family,peerAddr->ai_socktype,0))== INVALID_SOCKET)   {  
		freeaddrinfo(peerAddr);
		WSACleanup();
		return false;
	}

	if(connect(sockfd, peerAddr->ai_addr, peerAddr->ai_addrlen) == SOCKET_ERROR){
		closesocket(sockfd);
		freeaddrinfo(peerAddr);
		WSACleanup();
		return false;
	}
	freeaddrinfo(peerAddr);

	size_t stringlen = strlen(pkg->string)+1;
	int ret = send(sockfd, (const char *)&stringlen, sizeof(size_t), 0);
	if(SOCKET_ERROR == ret)
	{
		closesocket(sockfd);
		WSACleanup();
		return false;
	}

	size_t sendlen = 0, sendedlen = 0;
	while(sendedlen < stringlen)
	{
		sendlen = send(sockfd, pkg->string+sendedlen, stringlen-sendedlen, 0);
		if(SOCKET_ERROR == sendlen)
		{
			closesocket(sockfd);
		    WSACleanup();
		    return false;
		}

		sendedlen += sendlen;
	}

	WSACleanup();
	if(pkg->lpCallBack)
		pkg->lpCallBack(sockfd, pkg->string);

	closesocket(sockfd);
	free(pkg->ip);
	free(pkg->port);
	free(pkg->string);
	free(pkg);

	return 0;
}

void async_send_tcp_string(const char *ip, const char *port, const char *string, ASYNCSENDCALLBACKPROC lpCallBack)
{
	async_send_package *pkg = (async_send_package *)malloc(sizeof(Async_Send_Package));

	size_t iplen = strlen(ip) + 1, portlen = strlen(port) + 1, stringlen = strlen(string) + 1;
	pkg->ip = (char *)malloc(iplen);
	pkg->port = (char *)malloc(portlen);
	pkg->string = (char *)malloc(stringlen);

	strncpy_s(pkg->ip, iplen, ip, iplen);
	strncpy_s(pkg->port, portlen, port, portlen);
	strncpy_s(pkg->string, stringlen, string, stringlen);
	pkg->lpCallBack = lpCallBack;

	CloseHandle((HANDLE)_beginthreadex(NULL,0,async_send_tcp_string_thread,(LPVOID)pkg,0,NULL));
}


typedef struct UDP_Listener_Package
{
	bool enable_loop;
	SOCKET socket;
	LISTENERCALLBACKPROC callback_proc;

}udp_listener_package;

typedef struct UDP_Callback_Package
{
	char *recbuf;
	char *ip;
	int port;
	LISTENERCALLBACKPROC callback_proc;
}udp_callback_package;

unsigned int __stdcall udp_callback_thread(LPVOID pParam)
{
	udp_callback_package *pkg = (udp_callback_package *)pParam;

	if(pkg->callback_proc)
		pkg->callback_proc(pkg->recbuf, pkg->ip, pkg->port);

	free(pkg->recbuf);
	free(pkg->ip);
	free(pkg);

	return 0;
}

unsigned int __stdcall udp_listen_thread(LPVOID pParam)
{
	udp_listener_package *pkg = (udp_listener_package *)pParam;
	sockaddr_in6 preply_addr;
	int addr_len = sizeof(sockaddr_in6);

	bool is_myself = false;
	char **my_ipv4 = NULL, **my_ipv6 = NULL;
	int my_ipv4_num = 0, my_ipv6_num = 0;
	get_local_ip(&my_ipv4, &my_ipv6, &my_ipv4_num, &my_ipv6_num);

	while(TRUE)
	{
		int reclen = 0;
		char *ip = (char *)malloc(59);
		DWORD iplen = 59;
		int port = 0;
		char *recbuf = (char *)malloc(BUFFER_SIZE);

		memset(ip,0,59);
		memset(recbuf,0,BUFFER_SIZE);
		memset(&preply_addr,0,addr_len);

		if((reclen = recvfrom(pkg->socket, recbuf, BUFFER_SIZE, 0, (sockaddr*)&preply_addr, &addr_len))   ==   SOCKET_ERROR)
		{
			int ret = WSAGetLastError();
			break;
		}

		if(!memchr(recbuf, '\0', reclen))
		{
			free(recbuf);
			free(ip);
			recbuf = NULL;
			ip = NULL;
			continue;
		}
		
		if(preply_addr.sin6_family == PF_INET)       //IPv4情况
		{
			sockaddr_in v4_addr;
			memcpy(&v4_addr, &preply_addr, sizeof(sockaddr_in));
			char temp[59] = {0};
			WSAAddressToStringA((LPSOCKADDR)&v4_addr, sizeof(v4_addr), NULL, temp, &iplen);
			assert(strcmp(temp, ""));
			strncpy(ip, temp, strstr(temp, ":")-temp);
			char strport[6] = {0};
			strcpy_s(strport, strstr(temp, ":")+1);
			port = atoi(strport);

			if(!(pkg->enable_loop))
			{
			    //是否是自己的UDP包
			    for(int i=0; i<my_ipv4_num; i++)
			    {
				    if(!strcmp(ip, my_ipv4[i]))
				    {
					    is_myself = true;
					    break;
				    }
			    }
			}
		}
		else
		{
			char temp[59] = {0};
			WSAAddressToStringA((LPSOCKADDR)&preply_addr, sizeof(preply_addr), NULL, temp, &iplen); 
			assert(strcmp(temp, ""));
			strncpy(ip, temp+1, strstr(temp, "]")-temp-1);
			char strport[6] = {0};
			strcpy_s(strport, strstr(temp, "]")+2);
			port = atoi(strport);

			if(!(pkg->enable_loop))
			{
			    //是否是自己的UDP包
			    for(int i=0; i<my_ipv6_num; i++)
			    {
				    if(!strcmp(ip, my_ipv6[i]))
				    {
					    is_myself = true;
					    break;
				    }
			    }
			}
		}
		
		if((pkg->callback_proc) && (!is_myself))
		{
			udp_callback_package *upkg = (udp_callback_package *)malloc(sizeof(UDP_Callback_Package));
			upkg->recbuf = recbuf;
			upkg->ip = ip;
			upkg->port = port;
			upkg->callback_proc = pkg->callback_proc;

			CloseHandle((HANDLE)_beginthreadex(NULL,0,udp_callback_thread,(LPVOID)upkg,0,NULL));
		}
		else
		{
			free(recbuf);
			free(ip);
			recbuf = NULL;
			ip = NULL;
		}
	}
	
	release_socket_array_data((void ***)&my_ipv4);
	release_socket_array_data((void ***)&my_ipv6);
	closesocket(pkg->socket);
	free(pkg);
	WSACleanup();
	return 0;
}

SOCKET build_multicast_listener(const char *ip, const char *port, LISTENERCALLBACKPROC lpCallBack)
{
	SOCKET sockfd;
	WSADATA   WSAData;   
	if(SOCKET_ERROR == InitWSAData(WSAData))
		return SOCKET_ERROR;

	ADDRINFOA rinfot,*multicastAddr,*localAddr;
	memset(&rinfot,0,sizeof(rinfot));
	rinfot.ai_family=AF_UNSPEC;
	rinfot.ai_flags=AI_NUMERICHOST;                    //地址为字符串
	getaddrinfo(ip,NULL,&rinfot,&multicastAddr);
	rinfot.ai_family   = multicastAddr->ai_family;
	rinfot.ai_socktype = SOCK_DGRAM;
	rinfot.ai_flags    = AI_PASSIVE;                   //用于bind
	int iResult = getaddrinfo(NULL, port, &rinfot, &localAddr);
	if (iResult != 0)
        return SOCKET_ERROR;

	if   ((sockfd=socket(localAddr->ai_family,localAddr->ai_socktype,0))== INVALID_SOCKET)   {
		freeaddrinfo(localAddr);
		freeaddrinfo(multicastAddr);
		WSACleanup();
		return SOCKET_ERROR;
	}

	if   (bind(sockfd,localAddr->ai_addr,localAddr->ai_addrlen)   <   0)   {   
		closesocket(sockfd);
		freeaddrinfo(localAddr);
		freeaddrinfo(multicastAddr);
		WSACleanup();   
		return SOCKET_ERROR;         
	}   

	//IPv4情况
	if ( multicastAddr->ai_family  == PF_INET &&  
		multicastAddr->ai_addrlen == sizeof(struct sockaddr_in) )
	{
		struct ip_mreq multicastRequest;

		memcpy(&multicastRequest.imr_multiaddr,
			&((struct sockaddr_in*)(multicastAddr->ai_addr))->sin_addr,
			sizeof(multicastRequest.imr_multiaddr));
		multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

		//加入组播
		if ( setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &multicastRequest, sizeof(multicastRequest)) != 0 )
		{
			closesocket(sockfd);
		    freeaddrinfo(localAddr);
		    freeaddrinfo(multicastAddr);
		    WSACleanup();
			return SOCKET_ERROR;
		}
	}
	//IPv6情况
	else if ( multicastAddr->ai_family  == PF_INET6 &&
		multicastAddr->ai_addrlen == sizeof(struct sockaddr_in6) )
	{
		struct ipv6_mreq multicastRequest;

		memcpy(&multicastRequest.ipv6mr_multiaddr,
			&((struct sockaddr_in6*)(multicastAddr->ai_addr))->sin6_addr,
			sizeof(multicastRequest.ipv6mr_multiaddr));
		multicastRequest.ipv6mr_interface = 0;

		//加入组播
		if ( setsockopt(sockfd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*) &multicastRequest, sizeof(multicastRequest)) != 0 )
		{
			closesocket(sockfd);
		    freeaddrinfo(localAddr);
		    freeaddrinfo(multicastAddr);
		    WSACleanup();
			return SOCKET_ERROR;
		}
	}
	//异常情况
	else
	{
		closesocket(sockfd);
		freeaddrinfo(localAddr);
		freeaddrinfo(multicastAddr);
		WSACleanup();
	    return SOCKET_ERROR;
	}

	freeaddrinfo(localAddr);
	freeaddrinfo(multicastAddr);

	udp_listener_package *pkg = (udp_listener_package *)malloc(sizeof(UDP_Listener_Package));
	pkg->socket = sockfd;
	pkg->callback_proc = lpCallBack;
	pkg->enable_loop = false;

	CloseHandle((HANDLE)_beginthreadex(NULL,0,udp_listen_thread,(LPVOID)pkg,0,NULL));

	return sockfd;
}


SOCKET build_udp_listener(int family, const char *port, LISTENERCALLBACKPROC lpCallBack)
{
	SOCKET sockfd;
	WSADATA   WSAData;   
	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	DWORD status;
	ADDRINFOA Hints,*AddrInfo;

	if(SOCKET_ERROR == InitWSAData(WSAData))
		return SOCKET_ERROR;

	memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = family;             //AF_INET---IPv4，AF_UNSPEC---优先使用IPv6
    Hints.ai_socktype = SOCK_DGRAM;
	Hints.ai_protocol = IPPROTO_UDP;
    Hints.ai_flags = AI_PASSIVE;
	int iResult = getaddrinfo(NULL, port, &Hints, &AddrInfo);
	if (iResult != 0)
        return SOCKET_ERROR;

	if   ((sockfd=socket(AddrInfo->ai_family,AddrInfo->ai_socktype,0))== INVALID_SOCKET)   {
		freeaddrinfo(AddrInfo);
		WSACleanup();
		return SOCKET_ERROR;
	}

	if   (bind(sockfd,AddrInfo->ai_addr,AddrInfo->ai_addrlen)   <   0)   {   
		closesocket(sockfd);
		freeaddrinfo(AddrInfo);
		WSACleanup();   
		return SOCKET_ERROR;         
	}

	// 解决recvfrom error 10054
	// disable  new behavior using
	// IOCTL: SIO_UDP_CONNRESET
	status = WSAIoctl(sockfd, SIO_UDP_CONNRESET,
		&bNewBehavior, sizeof(bNewBehavior),
		NULL, 0, &dwBytesReturned,
		NULL, NULL);

	if (SOCKET_ERROR == status)
	{
		closesocket(sockfd);
		freeaddrinfo(AddrInfo);
		WSACleanup();   
		return SOCKET_ERROR;
	}

	freeaddrinfo(AddrInfo);

	udp_listener_package *pkg = (udp_listener_package *)malloc(sizeof(UDP_Listener_Package));
	pkg->socket = sockfd;
	pkg->callback_proc = lpCallBack;
	pkg->enable_loop = true;

	::CloseHandle((HANDLE)_beginthreadex(NULL,0,udp_listen_thread,(LPVOID)pkg,0,NULL));

	return sockfd;
}


typedef struct TCP_Listener_Package
{
	SOCKET socket;
	LISTENERWITHSOCKETCALLBACKPROC callback_proc;

}tcp_listener_package;

typedef struct Child_TCP_Listener_Package
{
	SOCKET socket;
	sockaddr_in6 preply_addr;
	LISTENERWITHSOCKETCALLBACKPROC callback_proc;
}child_tcp_listener_package;

unsigned int __stdcall tcp_child_listen_thread(LPVOID pParam)
{
	child_tcp_listener_package *child_pkg = (child_tcp_listener_package *)pParam;

	fd_set fds;
	struct timeval tv;

	FD_ZERO( &fds );
	FD_SET( child_pkg->socket, &fds );

	tv.tv_sec = MAX_TCP_WAIT_TIME;
	tv.tv_usec = 0;

	if((( select( child_pkg->socket + 1, &fds, 0, 0, &tv ) > 0 )
		&& FD_ISSET( child_pkg->socket, &fds ) != 0 ))
	{
		size_t stringlen = 0;
		int ret = recv(child_pkg->socket, (char *)&stringlen, sizeof(size_t), 0);
		if((SOCKET_ERROR == ret) || (stringlen > BUFFER_SIZE))
		{
			closesocket(child_pkg->socket);
			free(child_pkg);
			return 0;
		}

		char *recbuf = (char *)malloc(stringlen);
		size_t reclen = 0, recedlen = 0;
		while(recedlen < stringlen)
		{
			reclen = recv(child_pkg->socket, recbuf + recedlen, stringlen - recedlen, 0);
			if(SOCKET_ERROR == reclen)
				break;

			recedlen += reclen;
		}

		if(!memchr(recbuf, '\0', recedlen))
		{
			closesocket(child_pkg->socket);
			free(recbuf);
			free(child_pkg);
			return 0;
		}

		char ip[59] = {0};
		DWORD iplen = 59;
		int port =0;
		if(child_pkg->preply_addr.sin6_family == PF_INET)       //IPv4情况
		{
			sockaddr_in v4_addr;
			memcpy(&v4_addr, &(child_pkg->preply_addr), sizeof(sockaddr_in));
			char temp[59] = {0};
			WSAAddressToStringA((LPSOCKADDR)&v4_addr, sizeof(v4_addr), NULL, temp, &iplen);
			assert(strcmp(temp, ""));
			strncpy_s(ip, temp, strstr(temp, ":")-temp);
			char strport[6] = {0};
			strcpy_s(strport, strstr(temp, ":")+1);
			port = atoi(strport);
		}
		else
		{
			char temp[59] = {0};
			WSAAddressToStringA((LPSOCKADDR)&(child_pkg->preply_addr), sizeof(child_pkg->preply_addr), NULL, temp, &iplen); 
			assert(strcmp(temp, ""));
			strncpy_s(ip, temp+1, strstr(temp, "]")-temp-1);
			char strport[6] = {0};
			strcpy_s(strport, strstr(temp, "]")+2);
			port = atoi(strport);
		}
		if(child_pkg->callback_proc)
			child_pkg->callback_proc(child_pkg->socket, recbuf, ip, port);

		free(recbuf);
	}

	closesocket(child_pkg->socket);
	free(child_pkg);
	return 0;
}

unsigned int __stdcall tcp_listen_thread(LPVOID pParam)
{
	tcp_listener_package *pkg = (tcp_listener_package *)pParam;

	sockaddr_in6 preply_addr;
	int addr_len = sizeof(sockaddr_in6);

	while(TRUE)
	{
		memset(&preply_addr,0,addr_len);

		SOCKET new_socket;
		if((new_socket = accept(pkg->socket, (sockaddr*)&preply_addr, &addr_len)) == INVALID_SOCKET)
			continue;
		
		child_tcp_listener_package *child_pkg = (child_tcp_listener_package *)malloc(sizeof(Child_TCP_Listener_Package));
		child_pkg->socket = new_socket;
		child_pkg->preply_addr = preply_addr;
		child_pkg->callback_proc = pkg->callback_proc;

		::CloseHandle((HANDLE)_beginthreadex(NULL,0,tcp_child_listen_thread,(LPVOID)child_pkg,0,NULL));
	}
	
	closesocket(pkg->socket);
	free(pkg);
	WSACleanup();
	return 0;
}

SOCKET build_tcp_listener(int family, const char *port, LISTENERWITHSOCKETCALLBACKPROC lpCallBack)
{
	SOCKET sockfd;
	WSADATA   WSAData;   
	if(SOCKET_ERROR == InitWSAData(WSAData))
		return SOCKET_ERROR;

	ADDRINFOA Hints,*AddrInfo;
	memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family =family;             //AF_INET---IPv4，AF_UNSPEC---优先使用IPv6
    Hints.ai_socktype =SOCK_STREAM;
    Hints.ai_protocol=IPPROTO_TCP;
	Hints.ai_flags = AI_PASSIVE;
	int iResult = getaddrinfo(NULL, port, &Hints, &AddrInfo);
	if (iResult != 0)
        return SOCKET_ERROR;

	if   ((sockfd=socket(AddrInfo->ai_family,AddrInfo->ai_socktype,0))== INVALID_SOCKET)   {
		freeaddrinfo(AddrInfo);
		WSACleanup();
		return SOCKET_ERROR;
	}

	if   (bind(sockfd,AddrInfo->ai_addr,AddrInfo->ai_addrlen)   <   0)   {   
		closesocket(sockfd);
		freeaddrinfo(AddrInfo);
		WSACleanup();   
		return SOCKET_ERROR;         
	}
	freeaddrinfo(AddrInfo);

	if(SOCKET_ERROR == listen(sockfd, SOMAXCONN))
	{
		closesocket(sockfd);
		freeaddrinfo(AddrInfo);
		WSACleanup();   
		return SOCKET_ERROR;         
	}

	tcp_listener_package *pkg = (tcp_listener_package *)malloc(sizeof(TCP_Listener_Package));
	pkg->socket = sockfd;
	pkg->callback_proc = lpCallBack;

	::CloseHandle((HANDLE)_beginthreadex(NULL,0,tcp_listen_thread,(LPVOID)pkg,0,NULL));

	return sockfd;
}

int send_binary_data(const SOCKET sock, const unsigned char *bin, int binlen)
{
	if(SOCKET_ERROR == sock)
		return 0;

	int ret = send(sock, (const char *)&binlen, sizeof(int), 0);
	if((SOCKET_ERROR == ret) || (binlen == 0))
		return 0;

	int sendedlen = 0, sendlen = 0;
	while(sendedlen < binlen)
	{
		sendlen = send(sock, (const char *)bin+sendedlen, binlen-sendedlen, 0);
		if(SOCKET_ERROR == sendlen)
		    return sendedlen;

		sendedlen += sendlen;
	}

	return sendedlen;
}

int receive_binary_data(const SOCKET sock, const unsigned char *bin, int binlen)
{
	if(SOCKET_ERROR == sock)
		return 0;

	int size = 0;
	int ret = recv(sock, (char *)&size, sizeof(int), 0);
	if((SOCKET_ERROR==ret) || (size==0) || (size>binlen))
		return 0;

	int revedlen = 0, reclen = 0;
	while(revedlen < size)
	{
		reclen = recv(sock, (char *)bin + revedlen, size - revedlen, 0);
		if((0 == reclen) || (SOCKET_ERROR == reclen))
			break;

		revedlen += reclen;
	}

	return revedlen;
}

bool send_small_file_by_tcp_socket(const SOCKET sock, const char* filepath)
{
	if(SOCKET_ERROR == sock)
		return false;

	FILE *file = NULL;
	int size = 0;
	fopen_s(&file, filepath, "rb");

	if(!file)
		return false;
	else
	{
		fseek (file , 0 , SEEK_END);  
        size = ftell (file);  
        rewind (file);
	}

	if(size > SMALL_FILE_MAXSIZE)
	{
		if(file)
		    fclose(file);
		return false;
	}

	char *buffer = (char*) malloc(size);
    if (buffer == NULL)
    {  
        fclose(file);
		return false;
    }  
  
    size_t result = fread (buffer,1,size,file);
    if (result != size)  
    {  
		free(buffer);
        fclose(file);
		return false;
    }

	fclose(file);
	
	int sendlen = send_binary_data(sock, (const byte *)buffer, size);
	free(buffer);

	if(sendlen == size)
		return true;
	else
		return false;
}

bool receive_small_file_by_tcp_socket(const SOCKET sock, const char* filepath)
{
	if(SOCKET_ERROR == sock)
		return false;

	FILE *file = NULL;
	fopen_s(&file, filepath, "wb");
	if(!file)
		return false;

	int filesize = 0;
	int ret = recv(sock, (char *)&filesize, sizeof(int), 0);
	if((SOCKET_ERROR==ret) || (filesize==0) || (filesize>SMALL_FILE_MAXSIZE))
	{
		fclose(file);
		remove(filepath);
		return false;
	}

	char *buffer = (char*) malloc(filesize);
	if (buffer == NULL)  
	{
		fclose(file);
		remove(filepath);
        return false;
	}

	int revedlen = 0, reclen = 0;
	while(revedlen < filesize)
	{
		reclen = recv(sock, buffer + revedlen, filesize - revedlen, 0);
		if((0 == reclen) || (SOCKET_ERROR == reclen))
			break;

		revedlen += reclen;
	}

	int i = fwrite (buffer , 1 , revedlen , file );
	fclose(file);
	free(buffer);

	if(i != revedlen)
	{
		remove(filepath);
		return false;
	}
	return true;
}

bool get_file_by_http(const char* fileuri, const char* savedfilepath, const char* savedfilename)
{
	std::string url(fileuri);
	if(url.length() == 0)
		return false;

	std::string removeProtocol;  
    std::string serverName; 
	std::string requestFile;
    std::string sourcePath; 
	std::string fileName;
	std::string newfile = savedfilepath;
	newfile += "\\";
	newfile += savedfilename;

	int ret = 0;

	/////////////获取网络路径/////////////////////  
	ret = url.find("http://");  
	if(ret != std::string::npos)  
	{  
		removeProtocol = url.substr(strlen("http://"));  
	}  
	else  
	{  
		removeProtocol = url;  
	}

	ret = removeProtocol.find_first_of('/');  
	serverName = removeProtocol.substr(0,ret);  

	int net = removeProtocol.find_last_of('/');  
	sourcePath = removeProtocol.substr(ret,net-ret+1);  
	requestFile = removeProtocol.substr(ret);

	fileName = removeProtocol.substr(net+1);
	//////////////////////////////////////////////////////////////////

	WSADATA wsaData; 
	if(SOCKET_ERROR == InitWSAData(wsaData))
		return SOCKET_ERROR;

	IN_ADDR        iaHost;  
	LPHOSTENT    lpHostEntry;

	iaHost.s_addr = inet_addr(serverName.c_str());
	if (iaHost.s_addr == INADDR_NONE)  
	{  
		// Wasn't an IP address string, assume it is a name  
		lpHostEntry = gethostbyname(serverName.c_str());  
	}  
	else  
	{  
		// It was a valid IP address string  
		lpHostEntry = gethostbyaddr((const char *)&iaHost,   
			sizeof(struct in_addr), AF_INET);  
	}  
	if (lpHostEntry == NULL)  
	{
		return false;  
	}  

	//      
	// Create a TCP/IP stream socket  
	//  
	SOCKET    parmSock;      


	parmSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
	if (parmSock == INVALID_SOCKET)  
	{
		return false;  
	}  

	//  
	// Find the port number for the HTTP service on TCP  
	//
	LPSERVENT lpServEnt;  
	SOCKADDR_IN saServer;  

	lpServEnt = getservbyname("http", "tcp");  
	if (lpServEnt == NULL)
		saServer.sin_port = htons(80);  
	else  
		saServer.sin_port = lpServEnt->s_port;  

	//  
	// Fill in the rest of the server address structure  
	//  
	saServer.sin_family = AF_INET;  
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);  

	//  
	// Connect the socket  
	//  
	int nRet = connect(parmSock, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));  
	if (nRet == SOCKET_ERROR)
	{
		closesocket(parmSock);  
		return false;  
	}

    ///////////////////////////////////////////////////////////////
	char szBuffer[1024];
    //格式化http头  
    //第一行  
    sprintf(szBuffer,"GET %s HTTP/1.1\r\n",requestFile.c_str());  
      
    //第二行  
    strcat(szBuffer,"Host:");  
    strcat(szBuffer,serverName.c_str());  
    strcat(szBuffer,":");  
    strcat(szBuffer, "9090");  
    strcat(szBuffer,"\r\n");  
      
    //第三行  
    //strcat(szBuffer,"Accept:*/*");  
    strcat(szBuffer,"\r\n");  
  
  
    //////////////////////////  
    //第四行(可有可无)  
    strcat(szBuffer,"User-Agent:GeneralDownloadApplication");  
    strcat(szBuffer,"\r\n");  

    //第五行 非持久连接  
    strcat(szBuffer,"Connection:close");  
    strcat(szBuffer,"\r\n");  
    //最后一行  
    strcat(szBuffer,"\r\n");  
  
  
    nRet = send(parmSock, szBuffer, strlen(szBuffer), 0);  
    if (nRet == SOCKET_ERROR)  
    {
        closesocket(parmSock);      
        return false;  
    }

	//  
    // Receive the file contents and print to stdout  
    //  
    ///////////////////////////////////////////////////////////////  
        //取出http头,大小应该不超过1024个字节,只在第一次接收时进行处理  
    ///////////////////////////////////////////////////////////////  
    nRet = recv(parmSock, szBuffer, sizeof(szBuffer), 0);  
    if (nRet == SOCKET_ERROR)  
    { 
        closesocket(parmSock);
  
        WSACleanup();  
        return false;  
    }

	std::string strBuffer = szBuffer;
	std::string firstString = strBuffer.substr(0,strBuffer.find("\r\n"));
	int File_Exist = firstString.find("200");
	if(File_Exist == std::string::npos)
		return false;

	//获取文件内容开始位置  
	int i = 0;  
	while(1)  
	{  
		if((szBuffer[i]=='\r')&&(szBuffer[i+1]=='\n')&&(szBuffer[i+2]=='\r')&&(szBuffer[i+3]=='\n'))  
		{  
			break;  
		}  
		i++;
	}  
	//文件内容开始  
	int fileBegin = i+4;  

	FILE *file = NULL;
	fopen_s(&file, newfile.c_str(), "wb");
	if(!file)
	{
		closesocket(parmSock);
        WSACleanup();
		return false;
	}

	/////////////////////////////////////////////////////////////  
	for(i = fileBegin; i < nRet; i++)  
	{  
		fwrite(&szBuffer[i], 1, sizeof(szBuffer[i]), file);
	}

	while(1)  
	{  
		// Wait to receive, nRet = NumberOfBytesReceived  
		nRet = recv(parmSock, szBuffer, sizeof(szBuffer), 0);  


		if (nRet == SOCKET_ERROR)  
		{
			closesocket(parmSock);  
			fclose(file);  

			WSACleanup();  
			return false;  
		}  

		if (nRet == 0)  
			break;  
 
		fwrite(szBuffer, 1, nRet, file);  
	}  


	closesocket(parmSock);      
	fclose(file);  

	WSACleanup();
	return true;
}

int get_socket_family(const SOCKET sock)
{
	WSADATA   WSAData;   
	if(SOCKET_ERROR == InitWSAData(WSAData))
		return 0;

	sockaddr_in6 addr;
	memset(&addr, 0, sizeof(sockaddr_in6));
	int addrlen = sizeof(sockaddr_in6);
	if(getsockname(sock, (sockaddr *)&addr, &addrlen))
	{
		WSACleanup();
		return 0;
	}

	WSACleanup();
	return addr.sin6_family;
}

const char *get_socket_address(const SOCKET sock)
{
	sockaddr_in6 addr;
	memset(&addr, 0, sizeof(sockaddr_in6));
	int addrlen = sizeof(sockaddr_in6);
	if(getsockname(sock, (sockaddr *)&addr, &addrlen))
	{
		return NULL;
	}

	static char ip[51]={0};
	char temp[59] = {0};
	DWORD iplen = 59;
	if(addr.sin6_family == PF_INET)       //IPv4情况
	{
		sockaddr_in v4_addr;
		memcpy(&v4_addr, &(addr), sizeof(sockaddr_in));
		WSAAddressToStringA((LPSOCKADDR)&v4_addr, sizeof(v4_addr), NULL, temp, &iplen);
		assert(strcmp(temp, ""));
		strncpy_s(ip, temp, strstr(temp, ":")-temp);
	}
	else
	{
		WSAAddressToStringA((LPSOCKADDR)&(addr), sizeof(addr), NULL, temp, &iplen); 
		assert(strcmp(temp, ""));
		strncpy_s(ip, temp+1, strstr(temp, "]")-temp-1);
	}
	return ip;
}

int release_socket(SOCKET sock)
{
	if(SOCKET_ERROR == sock)
		return SOCKET_ERROR;

	int ret = closesocket(sock);
	WSACleanup();
	return ret;
}

void release_socket_data(void **p)
{
	if(*p)
		free(*p);
	*p=0;
}

void release_socket_array_data(void ***p_a)
{
	if(*p_a)
	{
		release_socket_data(&((*p_a)[0]));
		release_socket_data((void **)(&(*p_a)));
	}
}
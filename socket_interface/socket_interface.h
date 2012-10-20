#ifndef _SOCKET_INTERFACE_H_
#define _SOCKET_INTERFACE_H_

/*
 * Pull in WinSock2.h if necessary
 */
#ifndef _WINSOCK2API_
#include <WinSock2.h>
#endif /* _WINSOCK2API_ */

#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
#endif

typedef void (__stdcall* LISTENERCALLBACKPROC)(char const*, char const*, int);
typedef void (__stdcall* LISTENERWITHSOCKETCALLBACKPROC)(SOCKET, char const*, char const*, int);
typedef void (__stdcall* ASYNCSENDCALLBACKPROC)(SOCKET, char const*);

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TCP_WAIT_TIME 60                       //TCP��ȴ�ʱ��(��)
#define BUFFER_SIZE 1024                           //TCP��UDP�������հ�����󳤶�
#define SMALL_FILE_MAXSIZE 10*1024*1024

	//��ȡ�Լ���IP�������release_socket_array_data�ͷ�ipv4��ipv6
	bool get_local_ip(char ***ipv4, char ***ipv6, int *ipv4_num, int *ipv6_num);

	/*
	 *���ͺͽ����ַ���
	*/
	bool multicast_string(const char *ip, const char *port, int ttl, const char *string);                   //��������(BUFFER_SIZE)
	SOCKET build_multicast_listener(const char *ip, const char *port, LISTENERCALLBACKPROC lpCallBack);

	bool send_udp_string(const char *ip, const char *port, const char *string);         //��������(BUFFER_SIZE)
	bool send_udp_string_by_socket(const SOCKET sockfd, const char *ip, const char *port, const char *string);
	SOCKET build_udp_listener(int family, const char *port, LISTENERCALLBACKPROC lpCallBack);    //family:AF_INET---IPv4��AF_UNSPEC---����ʹ��IPv6

	bool send_tcp_string(const char *ip, const char *port, const char *string);
	bool send_tcp_string_without_closesocket(const char *ip, const char *port, const char *string, SOCKET *sock);
	void async_send_tcp_string(const char *ip, const char *port, const char *string, ASYNCSENDCALLBACKPROC lpCallBack);
	SOCKET build_tcp_listener(int family, const char *port, LISTENERWITHSOCKETCALLBACKPROC lpCallBack);

	/*
	 *���ͺͽ��ն���������
	*/
	// ����������
	int send_binary_data(const SOCKET sock, const unsigned char *bin, int binlen);
	int receive_binary_data(const SOCKET sock, const unsigned char *bin, int binlen);                // binLenΪҪ�������ݵ���󳤶�
	//С�ļ�
	bool send_small_file_by_tcp_socket(const SOCKET sock, const char* filepath);
	bool receive_small_file_by_tcp_socket(const SOCKET sock, const char* filepath);    //�ļ�����Ѵ�����ᱻ����

	bool get_file_by_http(const char* fileuri, const char* savedfilepath, const char* savedfilename);

	int get_socket_family(const SOCKET sock);                         //��ȡ�׽������ͣ�2--AF_INET,23--AF_INET6,0--fail
	const char *get_socket_address(const SOCKET sock);                //��ȡ�׽��ְ󶨵�IP
	int release_socket(SOCKET sock);
	void release_socket_data(void **p);
	void release_socket_array_data(void ***p_a);

#ifdef __cplusplus
}
#endif

#endif
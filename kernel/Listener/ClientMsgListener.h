#pragma once
#include <WinSock2.h>

#include "../Lock/SimpleCriticalSection.h"
#include "../json/json.h"

typedef void (*RECVUDPMSGCALLBACKPROC)(const char *, int, const Json::Value&, const char *);      //ip���˿ڣ���Ϣ���ݣ�mid���Է��ͻ����ϴ���Ϣ��id��

struct RecvUDPMsgItem
{
	BYTE msgType;
	RECVUDPMSGCALLBACKPROC callBack;
};

typedef void (*RECVTCPMSGCALLBACKPROC)(SOCKET, const char *, int, const Json::Value&);      //tcp�׽��֣�ip���˿ڣ���Ϣ����

struct RecvTCPMsgItem
{
	BYTE msgType;
	RECVTCPMSGCALLBACKPROC callBack;
};

class ClientMsgListener
{
public:
	ClientMsgListener()
	: v4UdpSock(SOCKET_ERROR), v6UdpSock(SOCKET_ERROR), v4TcpSock(SOCKET_ERROR), v6TcpSock(SOCKET_ERROR), v4Port(-1), v6Port(-1), hasListen(false)
    {}

	~ClientMsgListener()
	{}

	static ClientMsgListener *Instance();
	static void Release();

	bool Listen(int IPv4Port, int IPv6Port);                  //�˿ں��类ռ���Զ����������˿�
	SOCKET GetIPv4UDPSocket()
	{ return v4UdpSock; }
	SOCKET GetIPv6UDPSocket()
	{ return v6UdpSock; }
	int GetIPv4Port()
	{ return v4Port; }
	int GetIPv6Port()
	{ return v6Port; }

	/*
	 *�����ͻ�����Ϣ
	 *ACK��Ϣ��TCP��Ϣ������ظ�
	*/
	// UDP
	static void OnOnlineMsg(const char *ip, int port, const Json::Value &content, const char *mid);              //������Ϣ
	static void OnOnlineAckMsg(const char *ip, int port, const Json::Value &content, const char *mid);           //������ϢACK
	static void OnKeepOnlineMsg(const char *ip, int port, const Json::Value &content, const char *mid);          //����������Ϣ����������
	static void OnKeepOnlineAckMsg(const char *ip, int port, const Json::Value &content, const char *mid);       //����������Ϣ����������ACK
	static void OnOfflineMsg(const char *ip, int port, const Json::Value &content, const char *mid);             //������Ϣ������ظ���
	static void OnAddUserAskMsg(const char *ip, int port, const Json::Value &content, const char *mid);          //��Ӻ���������Ϣ
	static void OnReplyAddUserMsg(const char *ip, int port, const Json::Value &content, const char *mid);        //�ظ���Ӻ���������Ϣ
	static void OnChatWithTextMsg(const char *ip, int port, const Json::Value &content, const char *mid);        // ����������Ϣ
	static void OnNormalAckMsg(const char *ip, int port, const Json::Value &content, const char *mid);           // һ����ϢACK
	// TCP
	static void OnUserInfoUpdateMsg(SOCKET sock, const char *ip, int port, const Json::Value &content);          // ���º�����Ϣ��Ϣ
	static void OnGetFileMsg(SOCKET sock, const char *ip, int port, const Json::Value &content);                 // ��ȡ�ļ���Ϣ
	static void OnSendFileMsg(SOCKET sock, const char *ip, int port, const Json::Value &content);                // �����ļ���Ϣ

private:
	static ClientMsgListener *listener;
	static CriticalSection aCritSect;

	SOCKET v4UdpSock, v6UdpSock, v4TcpSock, v6TcpSock;
	int v4Port, v6Port;
	bool hasListen;

	static void __stdcall ListenerCallBack(char const*rec, char const*ip, int port);
	static void __stdcall ListenerWithSocketCallBack(SOCKET sock, char const*rec, char const*ip, int port);

	static bool ReceiveHeadImage(SOCKET sock, unsigned int uid);
	static void AnalyzeSource(const Json::Value &source);
};
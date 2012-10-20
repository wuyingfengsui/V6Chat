#pragma once
#include <WinSock2.h>

#include "../Lock/SimpleCriticalSection.h"
#include "../json/json.h"

typedef void (*RECVUDPMSGCALLBACKPROC)(const char *, int, const Json::Value&, const char *);      //ip，端口，消息内容，mid（对方客户端上此消息的id）

struct RecvUDPMsgItem
{
	BYTE msgType;
	RECVUDPMSGCALLBACKPROC callBack;
};

typedef void (*RECVTCPMSGCALLBACKPROC)(SOCKET, const char *, int, const Json::Value&);      //tcp套接字，ip，端口，消息内容

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

	bool Listen(int IPv4Port, int IPv6Port);                  //端口号如被占用自动换用其他端口
	SOCKET GetIPv4UDPSocket()
	{ return v4UdpSock; }
	SOCKET GetIPv6UDPSocket()
	{ return v6UdpSock; }
	int GetIPv4Port()
	{ return v4Port; }
	int GetIPv6Port()
	{ return v6Port; }

	/*
	 *其他客户端消息
	 *ACK消息及TCP消息均不需回复
	*/
	// UDP
	static void OnOnlineMsg(const char *ip, int port, const Json::Value &content, const char *mid);              //上线消息
	static void OnOnlineAckMsg(const char *ip, int port, const Json::Value &content, const char *mid);           //上线消息ACK
	static void OnKeepOnlineMsg(const char *ip, int port, const Json::Value &content, const char *mid);          //保持在线消息（心跳包）
	static void OnKeepOnlineAckMsg(const char *ip, int port, const Json::Value &content, const char *mid);       //保持在线消息（心跳包）ACK
	static void OnOfflineMsg(const char *ip, int port, const Json::Value &content, const char *mid);             //离线消息（无需回复）
	static void OnAddUserAskMsg(const char *ip, int port, const Json::Value &content, const char *mid);          //添加好友请求消息
	static void OnReplyAddUserMsg(const char *ip, int port, const Json::Value &content, const char *mid);        //回复添加好友请求消息
	static void OnChatWithTextMsg(const char *ip, int port, const Json::Value &content, const char *mid);        // 文字聊天消息
	static void OnNormalAckMsg(const char *ip, int port, const Json::Value &content, const char *mid);           // 一般消息ACK
	// TCP
	static void OnUserInfoUpdateMsg(SOCKET sock, const char *ip, int port, const Json::Value &content);          // 更新好友信息消息
	static void OnGetFileMsg(SOCKET sock, const char *ip, int port, const Json::Value &content);                 // 获取文件消息
	static void OnSendFileMsg(SOCKET sock, const char *ip, int port, const Json::Value &content);                // 发送文件消息

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
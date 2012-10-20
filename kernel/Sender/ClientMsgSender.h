#pragma once

#include <string>
#include <WinSock2.h>

#include "MsgSender.h"

class ClientMsgSender : public MsgSender
{
public:
	ClientMsgSender(const std::string &ip, int tcpPort, int udpPort)
		:ip(ip), tcpPort(tcpPort), udpPort(udpPort)
	{}

	void AddUserAskMsg();
	void ReplyAddUserMsg(bool agree);

	void TestOnlineMsg();
	void TestOnlineMsgAck(const std::string &mid);
	unsigned long KeepOnlineMsg();                                         // 需从userInfo剥离
	void KeepOnlineMsgAck(const std::string &mid);
	void Offline();

    void UpdateProfileMsg();
	bool GetFileMsg(const int fileId, const std::string &filePath);              // 同步消息，处理完成后返回

	void ChatWithTextMsg(const std::wstring &text, const std::string &sendTime);   // 启用新线程发送
	void NormalMsgAck(const std::string &mid);

private:
	std::string source;
	std::string ip;
	int tcpPort;
	int udpPort;

	static void __stdcall AfterOnlineMsg(bool success, const std::string &ip, const std::string &port, const std::string &msg);
	static void __stdcall AfterAddP2PUserMsg(bool success, const std::string &ip, const std::string &port, const std::string &msg);
	static void __stdcall AfterUpdateProfileMsg(SOCKET sock, char const *str);
	static void __stdcall AfterChatWithTextMsg(bool success, const std::string &ip, const std::string &port, const std::string &msg);

	static unsigned int __stdcall SendTextThread(LPVOID pParam);
	static bool SendHeadImage(SOCKET sock);

	static bool SendFileMsg(const std::string& ip, const int tcpPort, const int fileId, const std::string& source);     // 同步消息，处理完成后返回
};
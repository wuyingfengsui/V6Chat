#pragma once

#include <string>

#ifdef XMPP_INTERFACE_EXPORTS
class _declspec(dllexport) IXmppClientMessageListener
#else
class _declspec(dllimport) IXmppClientMessageListener
#endif
{
public:
	virtual void Connect() = 0;       // 已连接事件
	virtual void Disconnect() = 0;       // 连接断开/失败事件

	virtual void RecvText(            // 收到聊天消息事件
		int type,
		const std::string &serverName, 
		const std::string &peerAccount, 
		const std::string &body, 
		const std::string &subject
		) = 0;
};
#pragma once

#include <string>

#ifdef XMPP_INTERFACE_EXPORTS
class _declspec(dllexport) IXmppClientMessageListener
#else
class _declspec(dllimport) IXmppClientMessageListener
#endif
{
public:
	virtual void Connect() = 0;       // �������¼�
	virtual void Disconnect() = 0;       // ���ӶϿ�/ʧ���¼�

	virtual void RecvText(            // �յ�������Ϣ�¼�
		int type,
		const std::string &serverName, 
		const std::string &peerAccount, 
		const std::string &body, 
		const std::string &subject
		) = 0;
};
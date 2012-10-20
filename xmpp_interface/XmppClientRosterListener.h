#pragma once

#include <string>

#include "gloox\rostermanager.h"
#include "gloox\vcard.h"

#ifdef XMPP_INTERFACE_EXPORTS
class _declspec(dllexport) IXmppClientRosterListener
#else
class _declspec(dllimport) IXmppClientRosterListener
#endif
 {
 public:
	 virtual bool RecvRosterSubscriptionRequest(     // �յ������ϵ�������¼�
		const std::string &serverName, 
		const std::string &peerAccount,
		const std::string &msg,
		std::string &backName
		) = 0;

	virtual void RosterAdded(        // ��ϵ�˳ɹ�����¼�
		const std::string &serverName, 
		const std::string &peerAccount
		) = 0;

	virtual void RosterDeleted(        // ��ϵ��ɾ���¼�
		const std::string &serverName, 
		const std::string &peerAccount
		) = 0;

	virtual void RecvAllRosters(      // ���յ��������Ϸ�������ϵ���б�(�ڸյ�¼ʱ)
		const gloox::Roster& roster
		) = 0;

	virtual void RosterPresence(      // ��ϵ�˳�ϯ�¼�������״̬�ı䣩��photoType��0-�ޣ�1-URI��2-base64���ѽ��룩
		const gloox::RosterItem& item,
		gloox::Presence presence,
		const std::string& status,
		const int photoType,
		const std::string& photo
		) = 0;

	virtual void RecvVCard(           // ���յ��������Ϸ�������Ƭ��Ϣ
		const std::string &serverName, 
		const std::string &peerAccount,
		const std::string &formattedName,
		const std::string &nickName,
		const std::string &description,
		const std::string &birthDay,
		const gloox::VCard::Photo &photo
		) = 0;
 };
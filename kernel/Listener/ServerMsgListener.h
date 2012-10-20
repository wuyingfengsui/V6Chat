#pragma once

#include <string>
#include <list>
#include <Windows.h>

#include "../../xmpp_interface/XmppClient.h"

struct VCardInfo
{
	unsigned int uid;
	std::string account;         // utf-8��ʽ
	std::string formattedName;         // utf-8��ʽ
	std::wstring photoRelativePath;
};

class ServerMsgListener : public IXmppClientRosterListener, public IXmppClientMessageListener, public IXmppFileTransferListener
{
public:
	ServerMsgListener(unsigned int serverId) : id(serverId)
	{}

	virtual void Connect();
	virtual void Disconnect();

	virtual bool RecvRosterSubscriptionRequest(     // �յ������ϵ�������¼�
		const std::string &serverName, 
		const std::string &peerAccount,
		const std::string &msg,
		std::string &backName
		);

	virtual void RosterAdded(        // ��ϵ�˳ɹ�����¼�
		const std::string &serverName, 
		const std::string &peerAccount
		);

	virtual void RosterDeleted(        // ��ϵ��ɾ���¼�
		const std::string &serverName, 
		const std::string &peerAccount
		);

	virtual void RecvAllRosters(      // ���յ��������Ϸ�������ϵ���б�(�ڸյ�¼ʱ)
		const gloox::Roster& roster
		);

	virtual void RecvText(            // �յ�������Ϣ�¼�
		int type,
		const std::string &serverName, 
		const std::string &peerAccount, 
		const std::string &body, 
		const std::string &subject
		);

	virtual void RosterPresence(      // ��ϵ�˳�ϯ�¼�������״̬�ı䣩
		const gloox::RosterItem& item,
		gloox::Presence presence,
		const std::string& status,
		const int photoType,
		const std::string& photo
		);

	virtual void RecvVCard(           // ���յ��������Ϸ�������Ƭ��Ϣ
		const std::string &serverName, 
		const std::string &peerAccount,
		const std::string &formattedName,
		const std::string &nickName,
		const std::string &description,
		const std::string &birthDay,
		const gloox::VCard::Photo &photo
		);

	virtual bool RecvFileTransferRequest(
		const std::string &serverName, 
		const std::string &peerAccount, 
		const std::string& sid,
		const std::string& fileName, 
		long size, 
		const std::string& hash,
		const std::string& date, 
		const std::string& mimeType,
		const std::string& description,
		std::string &filePath
		);

	virtual void FileTransferEnd(
		bool success,
		const std::string &serverName,
		const std::string &peerAccount, 
		const std::string& sid
		);

	void SetId(unsigned int id)
	{
		this->id = id;
	}

	void GetSearchUsersResult(std::list<VCardInfo>& vCardList)
	{
		vCardList = this->vCardList;
	}

	void ClearSearchUsersResult()
	{
		this->vCardList.clear();
	}

private:
	unsigned int id;
	std::list<VCardInfo> vCardList;
};
#pragma once

#include <string>
#include <list>
#include <Windows.h>

#include "../../xmpp_interface/XmppClient.h"

struct VCardInfo
{
	unsigned int uid;
	std::string account;         // utf-8格式
	std::string formattedName;         // utf-8格式
	std::wstring photoRelativePath;
};

class ServerMsgListener : public IXmppClientRosterListener, public IXmppClientMessageListener, public IXmppFileTransferListener
{
public:
	ServerMsgListener(unsigned int serverId) : id(serverId)
	{}

	virtual void Connect();
	virtual void Disconnect();

	virtual bool RecvRosterSubscriptionRequest(     // 收到添加联系人请求事件
		const std::string &serverName, 
		const std::string &peerAccount,
		const std::string &msg,
		std::string &backName
		);

	virtual void RosterAdded(        // 联系人成功添加事件
		const std::string &serverName, 
		const std::string &peerAccount
		);

	virtual void RosterDeleted(        // 联系人删除事件
		const std::string &serverName, 
		const std::string &peerAccount
		);

	virtual void RecvAllRosters(      // 接收到服务器上发来的联系人列表(在刚登录时)
		const gloox::Roster& roster
		);

	virtual void RecvText(            // 收到聊天消息事件
		int type,
		const std::string &serverName, 
		const std::string &peerAccount, 
		const std::string &body, 
		const std::string &subject
		);

	virtual void RosterPresence(      // 联系人出席事件（在线状态改变）
		const gloox::RosterItem& item,
		gloox::Presence presence,
		const std::string& status,
		const int photoType,
		const std::string& photo
		);

	virtual void RecvVCard(           // 接收到服务器上发来的名片信息
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
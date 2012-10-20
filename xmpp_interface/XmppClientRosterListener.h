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
	 virtual bool RecvRosterSubscriptionRequest(     // 收到添加联系人请求事件
		const std::string &serverName, 
		const std::string &peerAccount,
		const std::string &msg,
		std::string &backName
		) = 0;

	virtual void RosterAdded(        // 联系人成功添加事件
		const std::string &serverName, 
		const std::string &peerAccount
		) = 0;

	virtual void RosterDeleted(        // 联系人删除事件
		const std::string &serverName, 
		const std::string &peerAccount
		) = 0;

	virtual void RecvAllRosters(      // 接收到服务器上发来的联系人列表(在刚登录时)
		const gloox::Roster& roster
		) = 0;

	virtual void RosterPresence(      // 联系人出席事件（在线状态改变），photoType：0-无，1-URI，2-base64（已解码）
		const gloox::RosterItem& item,
		gloox::Presence presence,
		const std::string& status,
		const int photoType,
		const std::string& photo
		) = 0;

	virtual void RecvVCard(           // 接收到服务器上发来的名片信息
		const std::string &serverName, 
		const std::string &peerAccount,
		const std::string &formattedName,
		const std::string &nickName,
		const std::string &description,
		const std::string &birthDay,
		const gloox::VCard::Photo &photo
		) = 0;
 };
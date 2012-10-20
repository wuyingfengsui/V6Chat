#pragma once
#include "UserInfo.h"
#include "../Sender/ServerMsgSender.h"

struct ServerUserInfo : UserInfo
{
	unsigned int serverid;
	std::string serveruid;      // utf-8格式

	unsigned long lastUpdateTime;   // 上次个人资料（vcard）更新时间

	ServerUserInfo(const unsigned int uid, const std::wstring &nick_name, const std::wstring &group_name,
		const unsigned short sex, const unsigned int age, const std::wstring &introduction, const std::wstring &headImagePath,
		const unsigned int serverid, std::string serveruid)
		:UserInfo(uid, nick_name, group_name, sex, age, introduction, headImagePath, "", 0, 0),
		serverid(serverid), serveruid(serveruid),
		lastUpdateTime(0)
	{
		Handle<MsgSender> hs(new ServerMsgSender(serverid, serveruid, uid));
		messageSender = hs;
	}

	ServerUserInfo(const unsigned int uid, const std::wstring &nick_name, const std::wstring &group_name,
		const unsigned short sex, const unsigned int age, const std::wstring &introduction, const std::wstring &headImagePath,
		const unsigned int serverid, std::string serveruid,
		UserState state)
		:UserInfo(uid, nick_name, group_name, sex, age, introduction, headImagePath, "", 0, 0, state),
		serverid(serverid), serveruid(serveruid)
	{
		Handle<MsgSender> hs(new ServerMsgSender(serverid, serveruid, uid));
		messageSender = hs;
	}

	~ServerUserInfo(){}

	bool CheckOnline()
	{
		return true;
	}

	void UpdateProfile();

	bool StoreInDB();
	bool Delete();
	virtual void GetLocation(std::wstring &location);
};
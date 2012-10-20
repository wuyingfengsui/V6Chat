#pragma once
#include "UserInfo.h"
#include "../Sender/ClientMsgSender.h"

struct P2PUserInfo : UserInfo
{
	unsigned long ackMsgid;

	P2PUserInfo(const unsigned int uid, const std::wstring &nick_name, const std::wstring &group_name,
		const unsigned short sex, const unsigned int age, const std::wstring &introduction, const std::wstring &headImagePath,
		const std::string ip, const int udpPort, const int tcpPort)
		:UserInfo(uid, nick_name, group_name, sex, age, introduction, headImagePath, ip, udpPort, tcpPort),
		ackMsgid(0)
	{
		Handle<MsgSender> hc(new ClientMsgSender(ip, tcpPort, udpPort));
		messageSender = hc;
	}

	~P2PUserInfo(){};

	bool CheckOnline();

	bool StoreInDB();
	bool Delete();

	void SetState(const UserState &state);
	virtual void GetLocation(std::wstring &location);
};
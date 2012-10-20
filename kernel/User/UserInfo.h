#pragma once
#include <string>
#include <queue>

#include "../V6ChatDef.h"
#include "../Handle.h"
#include "../Sender/MsgSender.h"

struct ChatMsg
{
	ChatMsg(const std::wstring &msg, const std::string &time, int msg_type)
		:msg(msg), time(time), msg_type(msg_type)
	{}

	std::wstring msg;
	std::string time;
	int msg_type;
};

struct UserInfo
{
	/* 需要存储的字段是否有更改 */
	bool hasChange;
	/*
	 *必填字段（不为空）
	*/
	unsigned int uid;
	std::wstring nick_name;
	std::wstring group_name;

	/*
	 *选填字段（可为空）
	*/
	unsigned short sex;                     //性别：0-保密，1-男，2-女
	unsigned int age;
	std::wstring introduction;
	std::wstring headImagePath;

	/*
	 *通讯字段（可为空）
	*/
	std::string ip;
	int udpPort, tcpPort;

	/*
	 *未读聊天内容队列
	*/
	std::queue<ChatMsg> unreadChatContent;

	/*
	 * 信息发送者
	 */
	Handle<MsgSender> messageSender;

	UserInfo(const unsigned int uid, const std::wstring &nick_name, const std::wstring &group_name,
		const unsigned short sex, const unsigned int age, const std::wstring &introduction, const std::wstring &headImagePath,
		const std::string ip, const int udpPort, const int tcpPort)
		:uid(uid), nick_name(nick_name), group_name(group_name),
		sex(sex), age(age), introduction(introduction), headImagePath(headImagePath),
		ip(ip), udpPort(udpPort), tcpPort(tcpPort),
		state(user_offline), lastActiveTime(0), hasChange(false)
	{}

	UserInfo(const unsigned int uid, const std::wstring &nick_name, const std::wstring &group_name,
		const unsigned short sex, const unsigned int age, const std::wstring &introduction, const std::wstring &headImagePath,
		const std::string ip, const int udpPort, const int tcpPort,
		UserState state)
		:uid(uid), nick_name(nick_name), group_name(group_name),
		sex(sex), age(age), introduction(introduction), headImagePath(headImagePath),
		ip(ip), udpPort(udpPort), tcpPort(tcpPort),
		state(state), lastActiveTime(0), hasChange(false)
	{}

	virtual void GetLocation(std::wstring &location) = 0;

	virtual ~UserInfo()
	{
		if(state == user_online)
			messageSender->Offline();
	}

	virtual bool CheckOnline() = 0;                          // 根据最后活动时间判断是否在线

	virtual bool StoreInDB() = 0;
	virtual bool Delete() = 0;

	void RecvText(int text_type, const std::wstring &text, const std::string &recordTime);                                 // 将聊天内容加入未读聊天内容队列
	bool RecordText(bool sendOrRecv, const std::wstring &text, std::string &recordTime);                                   // 将聊天内容记录到数据库

	virtual void SetState(const UserState &state)
	{ this->state = state; }
	virtual const UserState &GetState()
	{ return state; }

protected:
	/*
	 *用户状态字段
	*/
    UserState state;
	unsigned long lastActiveTime;
};
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
	/* ��Ҫ�洢���ֶ��Ƿ��и��� */
	bool hasChange;
	/*
	 *�����ֶΣ���Ϊ�գ�
	*/
	unsigned int uid;
	std::wstring nick_name;
	std::wstring group_name;

	/*
	 *ѡ���ֶΣ���Ϊ�գ�
	*/
	unsigned short sex;                     //�Ա�0-���ܣ�1-�У�2-Ů
	unsigned int age;
	std::wstring introduction;
	std::wstring headImagePath;

	/*
	 *ͨѶ�ֶΣ���Ϊ�գ�
	*/
	std::string ip;
	int udpPort, tcpPort;

	/*
	 *δ���������ݶ���
	*/
	std::queue<ChatMsg> unreadChatContent;

	/*
	 * ��Ϣ������
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

	virtual bool CheckOnline() = 0;                          // �������ʱ���ж��Ƿ�����

	virtual bool StoreInDB() = 0;
	virtual bool Delete() = 0;

	void RecvText(int text_type, const std::wstring &text, const std::string &recordTime);                                 // ���������ݼ���δ���������ݶ���
	bool RecordText(bool sendOrRecv, const std::wstring &text, std::string &recordTime);                                   // ���������ݼ�¼�����ݿ�

	virtual void SetState(const UserState &state)
	{ this->state = state; }
	virtual const UserState &GetState()
	{ return state; }

protected:
	/*
	 *�û�״̬�ֶ�
	*/
    UserState state;
	unsigned long lastActiveTime;
};
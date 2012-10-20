#pragma once

#include <stack>
#include <string>

#include "../Lock/SimpleCriticalSection.h"

struct UnreadMsg
{
	UnreadMsg(const int messageType, const std::wstring &replyName, const std::string &replyIP, const int replyPort, const std::wstring &message)
		:messageType(messageType), replyName(replyName), replyIP(replyIP), replyPort(replyPort), message(message)
	{}

	UnreadMsg(const int messageType,unsigned int replyId, const std::wstring &message)
		:messageType(messageType), replyId(replyId), message(message)
	{}

	UnreadMsg(const int messageType,unsigned int replyId, const std::wstring &replyName, const std::wstring &message)
		:messageType(messageType), replyId(replyId), replyName(replyName), message(message)
	{}

	int messageType;
	unsigned int replyId;
	std::wstring replyName;
	std::string replyIP;
	int replyPort;
	std::wstring message;
};

class UnreadMsgList
{
public:
	UnreadMsgList()
	{}
	~UnreadMsgList()
	{}

	static UnreadMsgList *Instance();
	static void Release();

	void AddUnreadMsg(const UnreadMsg &msg);
	bool DisplayMsg();
	bool GetUnreadMsg(int &msgType, unsigned int &replyId, std::wstring &replyName, std::string &replyIP, int &replyPort, std::wstring &msg);
	bool GetUnreadMsg(int &msgType, unsigned int &replyId, std::wstring &msg);
	bool GetUnreadMsg(int &msgType, std::wstring &msg);
	bool GetUnreadMsg(std::wstring &msg);
	void DeleteUnreadMsg();

private:
	static UnreadMsgList *msgList;
	static CriticalSection aCritSect;

	std::stack<UnreadMsg> messages;
};
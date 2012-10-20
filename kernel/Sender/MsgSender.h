#pragma once

#include <string>

class MsgSender
{
public:
	virtual void AddUserAskMsg() = 0;
	virtual void ReplyAddUserMsg(bool agree) = 0;
	virtual void Offline() = 0;

	virtual void UpdateProfileMsg() = 0;
	virtual void ChatWithTextMsg(const std::wstring &text, const std::string &sendTime) = 0;
};